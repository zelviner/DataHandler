#pragma once

#include "path.h"

#include <memory>
#include <qstring>

/// @brief 订单信息结构体
struct OrderInfo {
    std::string order_dir_name; // 订单文件夹名称
    std::string project_number; // 工程单号
    std::string order_number;   // 订单号
    std::string program_name;   // 项目名称
    std::string chip_model;     // 卡片型号
    std::string rf_code;        // 需求编码
    std::string script_package; // 脚本包
};



class Order {

  public:
    Order(std::shared_ptr<Path> path);
    ~Order();

    std::shared_ptr<OrderInfo> orderInfo(const std::string &order_dir_name);

  private:
    std::shared_ptr<Path>      path_;
    std::shared_ptr<OrderInfo> order_info_;
};