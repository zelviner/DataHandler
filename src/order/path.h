#pragma once

#include <string>

struct Path {
    Path(std::string datagram);
    ~Path();

    std::string datagram;       // 数据包路径
    std::string datagram_order; // 数据包订单路径

    std::string directory;  // 订单所在路径
    std::string order;      // 订单路径
    std::string data;       // 个人化数据路径
    std::string script;     // 脚本包路径
    std::string screenshot; // 截图路径
    std::string temp;       // 临时存放路径
    std::string tag_data;   // 标签数据路径
    std::string print;      // 打印路径
};