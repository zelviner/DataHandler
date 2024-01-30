#pragma once

#include <qstring>
#include <json/json.h>
#include <map>


/// @brief 首条个人化数据结构体
struct PersonDataInfo {
    QString         filename;
    QString         header;
    QString         data;
    QString         pin1;
    QString         ki;
    QString         op;
    zel::json::Json json_data;
};

class PersonData {

  public:
    PersonData(QString data_path);
    ~PersonData();

    PersonDataInfo *personDataInfo(QString &error);

  private:
    QString         data_path_;
    PersonDataInfo *person_data_info_;
};