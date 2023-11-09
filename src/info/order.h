#pragma once

#include "path.h"

#include <QString>
#include <string>

/// @brief 订单信息结构体
struct OrderInfo {
    QString order_dir_name;
    QString order_id;
    QString program_name;
    QString script_package;
    QString rf_code;
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