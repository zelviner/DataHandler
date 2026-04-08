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
        APP_CardReader(card_device_, reader_id_);
        APP_CardCallback(card_device_, &RunScript::callback_thunk, this);

        if (!APP_RunFile(card_device_, script_path_.c_str(), convert_)) {
            char error[1024];
            APP_GetLastError(card_device_, error, sizeof(error));
            emit failure(error);
            return;
        }
    }

  private:
    static void callback_thunk(const char *run_result, int len, void *user) {
        auto       *self = static_cast<RunScript *>(user);
        std::string str(run_result, len);

        auto pos = str.find("->");
        if (pos == std::string::npos) {
            QString result = QString::fromStdString(str);

            QMetaObject::invokeMethod(
                self,
                [self, result]() {
                    self->results_.enqueue(result); // 存队列
                    emit self->success(QString::fromStdString(self->script_name_), self->results_.dequeue());
                },
                Qt::QueuedConnection);
        } else {
            printf("%s\n", str.c_str());
        }
    }

  private:
    std::string     script_name_;
    std::string     script_path_;
    int             reader_id_;
    CARD_DEVICE     card_device_;
    QQueue<QString> results_; // 存储回调结果
    bool            convert_; // 转换为新脚本格式
};