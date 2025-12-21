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
class AkaAuth : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, START, AUTH, FINISH };

    AkaAuth(const std::shared_ptr<ScriptInfo> &script_info, const std::shared_ptr<PersonDataInfo> &person_data_info, int reader_id,
            const DATA_HANDLER &data_handler, bool convert = true)
        : script_info_(script_info)
        , person_data_info_(person_data_info)
        , reader_id_(reader_id)
        , data_handler_(data_handler)
        , convert_(convert) {}

  signals:
    // 信号函数，用于向外界发射信号
    void failure(AkaAuth::Type type, const QString &err_msg);
    void success(AkaAuth::Type type, const QString &duration, const QString &apdu_response);

  protected:
    void run() override {
        DH_CardReader(data_handler_, reader_id_);
        DH_CardCallback(data_handler_, &AkaAuth::callback_thunk, this);
        DH_PersoData(data_handler_, person_data_info_->path.c_str(), script_info_->has_ds);

        // 鉴权
        emit success(START, QString::fromStdString(duration_), "");

        // 计时 - 开始
        auto start = std::chrono::steady_clock::now();
        type_      = AUTH;

        // 执行清卡脚本
        if (!DH_Run(data_handler_, script_info_->aka_auth_path.c_str(), convert_)) {
            emit failure(type_, "鉴权失败, 请检查卡片是否完成个人化操作");
            char err_msg[1024];
            DH_GetLastError(data_handler_, err_msg, sizeof(err_msg));
            log_error(err_msg);
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
        auto       *self = static_cast<AkaAuth *>(user);
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
    DATA_HANDLER                    data_handler_;
    QQueue<QString>                 results_; // 存储回调结果
    Type                            type_;
    std::string                     duration_;
    bool                            convert_; // 转换为新脚本格式
};