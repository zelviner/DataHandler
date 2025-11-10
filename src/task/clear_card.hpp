#pragma once

#include "order/script.h"

#include <memory>
#include <qcoreapplication>
#include <qthread>
#include <zel/core.h>
#include <if_language/repl/repl_bridge.h>
#include <qqueue>

// 自定义的工作线程类
class ClearCard : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, START, CLEAR, FINISH };

    ClearCard(const std::shared_ptr<ScriptInfo> &script_info, const zel::json::Json &json_data, int reader_id, int xhlanguage_type, ApduProtocol protocol)
        : script_info_(script_info)
        , json_data_(json_data)
        , reader_id_(reader_id)
        , xhlanguage_type_(xhlanguage_type)
        , protocol_(protocol) {}

  signals:
    // 信号函数，用于向外界发射信号
    void failure(ClearCard::Type type, const QString &err_msg);
    void success(ClearCard::Type type, const QString &duration, const QString &apdu_response);

  protected:
    void run() override {
        json_data_["has_ds"] = script_info_->has_ds;
        auto personal_data   = json_data_.str();

        setCallback(&ClearCard::callback_thunk, this);

        // 清卡
        emit success(START, QString::fromStdString(duration_), "");

        // 计时 - 开始
        auto start  = std::chrono::steady_clock::now();
        bool result = false;
        type_       = CLEAR;
        if (xhlanguage_type_ == 0) {
            result = startCompiler(script_info_->clear_buffer.c_str(), personal_data.c_str(), reader_id_, protocol_);
        } else if (xhlanguage_type_ == 1) {
            result = startInterpreter(script_info_->clear_buffer.c_str(), personal_data.c_str(), reader_id_);
        }

        if (result == false) {
            emit failure(type_, "清卡脚本执行失败");
            return;
        }

        // 计时 - 结束
        auto end  = std::chrono::steady_clock::now();
        duration_ = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 完成
        emit success(FINISH, QString::fromStdString(duration_), "");
    }

  private:
    static void callback_thunk(const char *run_result, int len, void *user) {
        auto   *self = static_cast<ClearCard *>(user);
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
    ApduProtocol                protocol_;
    QQueue<QString>             results_; // 存储回调结果
    Type                        type_;
    std::string                 duration_;
};