#pragma once

#include <string>

struct DmsOrderInfo {
    std::string order_no;              // 订单号
    int         order_id;              // 订单ID
    int         batch_list_id;         // 批次列表ID
    std::string perso_data_table_name; // 个人化数据表名
    std::string perso_script_path;     // 个人化脚本路径
    std::string verify_script_path;    // 检测脚本路径
    std::string clear_script_path;     // 清卡脚本路径
};