#pragma once

#include "order/script.h"

#include <cstring>
#include <zel/zel.h>
#include <memory>
#include <qcoreapplication>
#include <qthread>
#include <xhlanguage/repl/repl_bridge.h>
#include <qqueue>

// 自定义的工作线程
class WriteCard : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, BARE_ATR, PREPERSONAL, WHITE_ATR, POSTPERSONAL, CHECK, FINISHED_ATR, FINISH };

    WriteCard(const std::shared_ptr<ScriptInfo> &script_info, const zel::json::Json &json_data, int reader_id, int xhlanguage_type)
        : script_info_(script_info)
        , json_data_(json_data)
        , reader_id_(reader_id)
        , xhlanguage_type_(xhlanguage_type) {}
    ~WriteCard() {}

  signals:
    // 信号函数，用于向外界发射信号
    void failure(WriteCard::Type type, const QString &err_msg);
    void success(WriteCard::Type type, const QString &duration_, const QString &atr = "");

  protected:
    void run() override {

        json_data_["has_ds"] = script_info_->has_ds;
        auto personal_data   = json_data_.str();

        setCallback(&WriteCard::callbackThunk, this);

        // 获取裸卡 ATR
        char error_message[1024];
        type_     = BARE_ATR;
        duration_ = "";
        startCompiler("RST()", "", reader_id_, error_message, sizeof(error_message));

        // 预个人化
        auto start       = std::chrono::steady_clock::now();
        bool success_run = false;
        type_            = PREPERSONAL;
        memset(error_message, 0, sizeof(error_message));
        if (xhlanguage_type_ == 0) {
            success_run = startCompiler(script_info_->person_buffer.c_str(), personal_data.c_str(), reader_id_, error_message, sizeof(error_message));
        } else if (xhlanguage_type_ == 1) {
            success_run = startInterpreter(script_info_->person_buffer.c_str(), personal_data.c_str(), reader_id_);
        }

        if (success_run == false) {
            emit failure(type_, error_message);
            return;
        }

        // 计时 - 结束
        auto end  = std::chrono::steady_clock::now();
        duration_ = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 获取白卡 ATR
        memset(error_message, 0, sizeof(error_message));
        type_ = WHITE_ATR;
        startCompiler("RST()", "", reader_id_, error_message, sizeof(error_message));

        // 后个人化
        start = std::chrono::steady_clock::now();
        memset(error_message, 0, sizeof(error_message));
        type_ = POSTPERSONAL;
        if (xhlanguage_type_ == 0) {
            success_run = startCompiler(script_info_->post_person_buffer.c_str(), personal_data.c_str(), reader_id_, error_message, sizeof(error_message));
        } else if (xhlanguage_type_ == 1) {
            success_run = startInterpreter(script_info_->post_person_buffer.c_str(), personal_data.c_str(), reader_id_);
        }

        if (success_run == false) {
            emit failure(type_, error_message);
            return;
        }

        // 计时 - 结束
        end       = std::chrono::steady_clock::now();
        duration_ = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 获取成卡 ATR
        type_ = FINISHED_ATR;
        memset(error_message, 0, sizeof(error_message));
        startCompiler("RST()", "", reader_id_, error_message, sizeof(error_message));

        // 检测
        start = std::chrono::steady_clock::now();
        memset(error_message, 0, sizeof(error_message));
        type_ = CHECK;
        if (xhlanguage_type_ == 0) {
            success_run = startCompiler(script_info_->check_buffer.c_str(), personal_data.c_str(), reader_id_, error_message, sizeof(error_message));
        } else if (xhlanguage_type_ == 1) {
            success_run = startInterpreter(script_info_->check_buffer.c_str(), personal_data.c_str(), reader_id_);
        }

        if (success_run == false) {
            emit failure(type_, error_message);
            return;
        }

        // 计时 - 结束
        end       = std::chrono::steady_clock::now();
        duration_ = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 完成
        emit success(FINISH, QString::fromStdString(duration_));
    }

  private:
    static void callbackThunk(const char *run_result, int len, void *user) {
        auto   *self = static_cast<WriteCard *>(user);
        QString str  = QString::fromUtf8(run_result, len);

        QMetaObject::invokeMethod(
            self,
            [self, str]() {
                self->results_.enqueue(str); // 存队列
                emit self->success(self->type_, QString::fromStdString(self->duration_), self->results_.dequeue());
            },
            Qt::QueuedConnection);
    }

  private:
    std::shared_ptr<ScriptInfo> script_info_;
    zel::json::Json             json_data_;
    int                         reader_id_;
    int                         xhlanguage_type_;
    QQueue<QString>             results_; // 存储回调结果
    Type                        type_;
    std::string                 duration_;
};