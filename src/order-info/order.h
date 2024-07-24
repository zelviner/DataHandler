#pragma once

#include "path.h"

#include <memory>
#include <qstring>

/// @brief 订单信息结构体
struct OrderInfo {
    QString order_dir_name; // 订单文件夹名称
    QString project_number; // 工程单号
    QString order_number;   // 订单号
    QString program_name;   // 项目名称
    QString chip_model;     // 卡片型号
    QString rf_code;        // 需求编码
    QString script_package; // 脚本包
};

class Order {

  public:
    Order(std::shared_ptr<Path> path);
    ~Order();

    std::shared_ptr<OrderInfo> orderInfo(QString &error);

  private:
    std::shared_ptr<Path>      path_;
    std::shared_ptr<OrderInfo> order_info_;
};