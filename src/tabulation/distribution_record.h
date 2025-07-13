#pragma once

#include <string>
#include <vector>

// 表头信息
struct DistributionRecordHeader {
    std::string              order_no;       // 订单号
    int                      order_quantity; // 数量
};

// 表数据信息
struct DistributionRecordData {
    std::string filename;    // 文件名
    int         quantity;    // 数量
    std::string start_iccid; // 起始ICCID
    std::string end_iccid;   // 终止ICCID
};

// 数据分配表
struct DistributionRecord {
    DistributionRecordHeader            header; // 表头信息
    std::vector<DistributionRecordData> datas;  // 表数据信息
};