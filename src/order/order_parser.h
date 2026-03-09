#pragma once

#include <mupdf/fitz.h>
#include <memory>
#include <string>

/// @brief 订单信息结构体
struct OrderInfo {
    std::string order_number;   // 订单编号
    int         quantity;       // 订单数量
    std::string project_name;   // 项目名称
    std::string product_type;   // 产品类型
    std::string rf_code;        // 项目需求编码
    std::string script_package; // 项目脚本包
    std::string chip_model;     // 芯片型号
};

class OrderParser {
  public:
    std::shared_ptr<OrderInfo> parse(const std::string &pdf);

  private:
    std::shared_ptr<OrderInfo> read_form_fields(fz_context *ctx, fz_document *doc);
};