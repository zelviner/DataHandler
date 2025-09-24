#pragma once

#include "order/order.h"
#include "order/path.h"
#include "order/person_data.h"
#include "order/script.h"

#include <memory>
#include <qcoreapplication>
#include <qdebug>
#include <qthread>

using namespace zel::filesystem;

// 自定义的工作线程类
class HandleOrder : public QThread {
    Q_OBJECT

  public:
    HandleOrder(std::shared_ptr<Path> &path)
        : path_(path) {}

  signals:
    // 信号函数，用于向外界发射信号
    void failure(const QString &err_msg);
    void success(std::shared_ptr<OrderInfo> order_info, std::shared_ptr<PersonDataInfo> person_data_info, std::shared_ptr<ScriptInfo> script_info);

  protected:
    void run() override {
        Order order(path_);
        if (FilePath::isFile(path_->datagram)) {
            // 订单预处理
            if (!order.preProcessing()) {
                emit failure("订单预处理失败，请查看日志文件 'DataHandler.log' 了解详细信息");
                return;
            }
        } else {
            path_->datagram_order = path_->datagram;
        }

        // 判断是否修改订单
        if (path_->datagram_order != path_->order) {
            if (!order.modify()) {
                emit failure("修改工程单号和订单号失败，请查看日志文件 'DataHandler.log' 了解详细信息");
                return;
            }
        }

        // 订单处理
        if (!order.processing()) {
            emit failure("订单处理失败，请查看日志文件 'DataHandler.log' 了解详细信息");
            return;
        }

        // 获取订单信息
        auto order_info       = order.orderInfo();
        auto person_data_info = order.personDataInfo();
        auto script_info      = order.scriptInfo();

        emit success(order_info, person_data_info, script_info);
    }

  private:
    std::shared_ptr<Path> path_;
};