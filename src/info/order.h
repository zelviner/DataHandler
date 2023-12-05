#pragma once

#include "path.h"

#include <QString>
#include <string>

/// @brief 订单信息结构体
struct OrderInfo {
    QString order_dir_name; // 订单文件夹名称
    QString order_id;       // 工程单号
    QString program_name;   // 项目名称
    QString rf_code;        // 需求编码 + 卡片型号
    QString script_package; // 脚本包
};

class Order {

  public:
    Order(Path *path);
    ~Order();

    OrderInfo *orderInfo(QString &error);

  private:
    Path      *path_;
    OrderInfo *order_info_;
};