#pragma once

#include <memory>
#include <qstring>
#include <zel/zel.h>

/// @brief 首条个人化数据结构体
struct PersonDataInfo {
    std::string     filename; // 个人化文件名
    std::string     header;   // 头信息
    std::string     data;     // 数据
    std::string     pin1;
    std::string     ki;
    std::string     op;
    zel::json::Json json_data;
};

class PersonData {

  public:
    PersonData(const std::string &data_path);
    ~PersonData();

    std::shared_ptr<PersonDataInfo> personDataInfo();

  private:
    zel::json::Json json_data();

  private:
    std::string                     data_path_;
    std::shared_ptr<PersonDataInfo> person_data_info_;
};