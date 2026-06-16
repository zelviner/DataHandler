#pragma once

#include "order/script.h"
#include "order/person_data.h"

#include <memory>
#include <qcoreapplication>
#include <qthread>
#include <zel/core.h>
#include <card_device/card_device.h>
#include <qqueue>

// 自定义的工作线程类
class ClearCard : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, START, CLEAR, FINISH };

    ClearCard(const std::shared_ptr<ScriptInfo> &script_info, const std::shared_ptr<PersonDataInfo> &person_data_info, int reader_id,
              const CARD_DEVICE &card_device, bool convert = true)
        : script_info_(script_info)
        , person_data_info_(person_data_info)
        , reader_id_(reader_id)
        , card_device_(card_device)
        , convert_(convert) {}

  signals:
    // 信号函数，用于向外界发射信号
    void failure(ClearCard::Type type, const QString &err_msg);
    void success(ClearCard::Type type, const QString &duration, const QString &apdu_response);

  protected:
    void run() override {
        APP_CardReader(card_device_, reader_id_);
        APP_CardCallback(card_device_, &ClearCard::callback_thunk, this);
        APP_PersoDataFile(card_device_, person_data_info_->path.c_str(), script_info_->has_ds);

        char code[1024 * 200] = {0};
        if (convert_) {
            if (!APP_ScriptConvertTelecom(card_device_, script_info_->clear_buffer.c_str(), code, sizeof(code))) {
                emit failure(START, "脚本转换失败, 请检查脚本");
                char err_msg[1024];
                APP_GetLastError(card_device_, err_msg, sizeof(err_msg));
                log_error(err_msg);
                return;
            }
        } else {
            snprintf(code, sizeof(code), "%s", script_info_->clear_buffer.c_str());
        }

        // 清卡
        emit success(START, QString::fromStdString(duration_), "");

        // 计时 - 开始
        auto start = std::chrono::steady_clock::now();
        type_      = CLEAR;

        // 执行清卡脚本
        if (!APP_RunCode(card_device_, code)) {
            emit failure(type_, "清卡脚本执行失败");
            char error[1024];
            APP_GetLastError(card_device_, error, sizeof(error));
            log_error(error);
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
        auto           *self = static_cast<ClearCard *>(user);
        zel::json::Json json;
        json.load(run_result, len);

        std::string show;
        auto        event = json["event"];
        if (event == "script.print") {
            show = json["data"]["message"].asString();
        } else if (event == "card.reset") {
            show = "RST -> " + json["data"]["atr"].asString();
        } else if (event == "card.apdu") {
            show = json["data"]["command"].asString() + " -> " + json["data"]["response"].asString();
        }

        log_info("%s", show.c_str());
        QString result = QString::fromStdString(show);
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
    bool                            convert_; // 转换为新脚本格式
};