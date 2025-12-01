#pragma once

#include "order/script.h"
#include "order/person_data.h"

#include <memory>
#include <qcoreapplication>
#include <qthread>
#include <zel/core.h>
#include <card_device/data_handler/data_handler.h>
#include <qqueue>

// 自定义的工作线程类
class ClearCard : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, START, CLEAR, FINISH };

    ClearCard(const std::shared_ptr<ScriptInfo> &script_info, const std::shared_ptr<PersonDataInfo> &person_data_info, int reader_id,
              const std::shared_ptr<card_device::DataHandler> &data_handler)
        : script_info_(script_info)
        , person_data_info_(person_data_info)
        , reader_id_(reader_id)
        , data_handler_(data_handler) {}

  signals:
    // 信号函数，用于向外界发射信号
    void failure(ClearCard::Type type, const QString &err_msg);
    void success(ClearCard::Type type, const QString &duration, const QString &apdu_response);

  protected:
    void run() override {
        data_handler_->selectCardReader(reader_id_);
        data_handler_->cardCallback(&ClearCard::callback_thunk, this);

        data_handler_->persoData(person_data_info_->path, script_info_->has_ds);

        // 清卡
        emit success(START, QString::fromStdString(duration_), "");

        // 计时 - 开始
        auto start = std::chrono::steady_clock::now();
        type_      = CLEAR;

        // 执行清卡脚本
        if (!data_handler_->run(script_info_->clear_path)) {
            emit failure(type_, "清卡脚本执行失败");
            log_error(data_handler_->error().c_str());
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
        auto       *self = static_cast<ClearCard *>(user);
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
    std::shared_ptr<ScriptInfo>               script_info_;
    std::shared_ptr<PersonDataInfo>           person_data_info_;
    int                                       reader_id_;
    std::shared_ptr<card_device::DataHandler> data_handler_;
    QQueue<QString>                           results_; // 存储回调结果
    Type                                      type_;
    std::string                               duration_;
};