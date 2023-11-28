#pragma once
#include <QString>
#include <map>


/// @brief 首条个人化数据结构体
struct PersonDataInfo {
    QString filename;
    QString header;
    QString data;
    QString pin1;
    QString op;
    QString ki;
    std::string json_data;
};

class PersonData {

  public:
    PersonData(QString data_path);
    ~PersonData();

    PersonDataInfo *personDataInfo(QString &error);

  private:
    QString                    data_path_;
    PersonDataInfo            *person_data_info_;
    std::map<QString, QString> m_data_;
};