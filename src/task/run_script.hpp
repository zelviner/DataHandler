#pragma once

#include <qcoreapplication>
#include <qthread>
#include <zel/core.h>
#include <card_device/card_device.h>
#include <qqueue>

// 自定义的工作线程类
class RunScript : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, START, CLEAR, FINISH };

    RunScript(const std::string &script_name, const std::string &script_path, int reader_id, const CARD_DEVICE &card_device, bool convert = false)
        : script_name_(script_name)
        , script_path_(script_path)
        , reader_id_(reader_id)
        , card_device_(card_device)
        , convert_(convert) {}

  signals:
    // 信号函数，用于向外界发射信号
    void failure(const QString &err_msg);
    void success(const QString &script_name, const QString &apdu_response);

  protected:
    void run() override {
        if (!APP_CardReader(card_device_, reader_id_)) {
            char error[1024];
            APP_GetLastError(card_device_, error, sizeof(error));
            emit failure(error);
            return;
        }

        if (!APP_CardCallback(card_device_, &RunScript::callback_thunk, this)) {
            char error[1024];
            APP_GetLastError(card_device_, error, sizeof(error));
            emit failure(error);
            return;
        }

        if (!APP_RunFile(card_device_, script_path_.c_str(), convert_)) {
            char error[1024];
            APP_GetLastError(card_device_, error, sizeof(error));
            emit failure(error);
            return;
        }
    }

  private:
    static void callback_thunk(const char *run_result, int len, void *user) {
        auto           *self = static_cast<RunScript *>(user);
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
                emit self->success(QString::fromStdString(self->script_name_), self->results_.dequeue());
            },
            Qt::QueuedConnection);
    }

  private:
    std::string     script_name_;
    std::string     script_path_;
    int             reader_id_;
    CARD_DEVICE     card_device_;
    QQueue<QString> results_; // 存储回调结果
    bool            convert_; // 转换为新脚本格式
};