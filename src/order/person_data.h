#pragma once

#include <memory>
#include <qstring>
#include <zel/core.h>

/// @brief 首条个人化数据结构体
struct PersonDataInfo {
    std::string     filename; // 个人化文件名
    std::string     prefix;   // 个人化文件前缀
    std::string     path;     // 个人化文件路径
    std::string     header;   // 头信息
    std::string     data;     // 数据
    zel::json::Json json_data;
};

class PersonData {

  public:
    PersonData(const std::string &data_file, const std::string &dest_path);
    ~PersonData();

    std::shared_ptr<PersonDataInfo> personDataInfo();

  private:
    bool            split_file_stream(const std::string &dest_path);
    bool            write(const std::string &filename, const std::string context);
    zel::json::Json json_data();

  private:
    std::string                     data_file_;
    std::string                     dest_path_;
    std::shared_ptr<PersonDataInfo> person_data_info_;
};