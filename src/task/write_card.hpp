#pragma once

#include "order/script.h"
#include "order/person_data.h"

#include <memory>
#include <qcoreapplication>
#include <qthread>
#include <card_device/card_device.h>
#include <qqueue>
#include <zel/core.h>

// 自定义的工作线程
class WriteCard : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, BARE_ATR, PREPERSONAL, WHITE_ATR, POSTPERSONAL, CHECK, FINISHED_ATR, FINISH };

    WriteCard(const std::shared_ptr<ScriptInfo> &script_info, const std::shared_ptr<PersonDataInfo> &person_data_info, int reader_id,
              const CARD_DEVICE &card_device)
        : script_info_(script_info)
        , person_data_info_(person_data_info)
        , reader_id_(reader_id)
        , card_device_(card_device) {}
    ~WriteCard() {}

  signals:
    // 信号函数，用于向外界发射信号
    void failure(WriteCard::Type type, const QString &err_msg);
    void success(WriteCard::Type type, const QString &duration_, const QString &atr = "");

  protected:
    void run() override {
        APP_CardReader(card_device_, reader_id_);
        APP_CardCallback(card_device_, &WriteCard::callback_thunk, this);
        APP_PersoData(card_device_, person_data_info_->path.c_str(), script_info_->has_ds);

        // 获取裸卡 ATR
        type_ = BARE_ATR;
        char atr[1024];
        APP_ResetCardReader(card_device_, true, atr, sizeof(atr));
        emit success(type_, "", atr);

        // 预个人化
        auto start = std::chrono::steady_clock::now();
        type_      = PREPERSONAL;
        if (!APP_Run(card_device_, script_info_->person_path.c_str(), true)) {
            emit failure(type_, "预个人化脚本执行失败");
            char err_msg[1024];
            APP_GetLastError(card_device_, err_msg, sizeof(err_msg));
            log_error(err_msg);
            return;
        }

        // 计时 - 结束
        auto end  = std::chrono::steady_clock::now();
        duration_ = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";
        emit success(type_, QString::fromStdString(duration_), QString::fromStdString(atr));

        // 获取白卡 ATR
        type_ = WHITE_ATR;
        memset(atr, 0, sizeof(atr));
        APP_ResetCardReader(card_device_, true, atr, sizeof(atr));
        emit success(type_, QString::fromStdString(duration_), QString::fromStdString(atr));

        // 后个人化
        start = std::chrono::steady_clock::now();
        type_ = POSTPERSONAL;
        if (!APP_Run(card_device_, script_info_->post_person_path.c_str(), true)) {
            emit failure(type_, "后个人化脚本执行失败");
            char err_msg[1024];
            APP_GetLastError(card_device_, err_msg, sizeof(err_msg));
            log_error(err_msg);
            return;
        }

        // 计时 - 结束
        end       = std::chrono::steady_clock::now();
        duration_ = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";
        emit success(type_, QString::fromStdString(duration_), QString::fromStdString(atr));

        // 获取成卡 ATR
        type_ = FINISHED_ATR;
        memset(atr, 0, sizeof(atr));
        APP_ResetCardReader(card_device_, true, atr, sizeof(atr));
        emit success(type_, QString::fromStdString(duration_), QString::fromStdString(atr));

        // 检测
        start = std::chrono::steady_clock::now();
        type_ = CHECK;
        if (!APP_Run(card_device_, script_info_->check_path.c_str(), true)) {
            emit failure(type_, "检测脚本执行失败");
            char err_msg[1024];
            APP_GetLastError(card_device_, err_msg, sizeof(err_msg));
            log_error(err_msg);
            return;
        }

        // 计时 - 结束
        end       = std::chrono::steady_clock::now();
        duration_ = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";
        emit success(FINISH, QString::fromStdString(duration_));
    }

  private:
    static void callback_thunk(const char *run_result, int len, void *user) {
        auto *self = static_cast<WriteCard *>(user);

        std::string str(run_result, len);

        auto pos  = str.find("->");
        auto apdu = str.substr(0, pos - 1);
        auto rsp  = str.substr(pos + 3, str.size() - pos - 4);

        if (apdu.size() > 70) {
            // 取前20个字节
            apdu = apdu.substr(0, 30) + "...";
        }

        if (rsp.size() > 70) {
            // 取前20个字节
            rsp = rsp.substr(0, 20) + "...";
        }

        QString result = QString::fromStdString(apdu) + " -> " + QString::fromStdString(rsp);

        log_info(run_result);

        QMetaObject::invokeMethod(
            self,
            [self, result]() {
                self->results_.enqueue(result); // 存队列
                emit self->success(self->type_, QString::fromStdString(self->duration_), self->results_.dequeue());
            },
            Qt::QueuedConnection);
    }

  private:
    std::shared_ptr<ScriptInfo>     script_info_;
    std::shared_ptr<PersonDataInfo> person_data_info_;
    int                             reader_id_;
    CARD_DEVICE                     card_device_;
    QQueue<QString>                 results_; // 存储回调结果
    Type                            type_;
    std::string                     duration_;
};