#include "person_data.h"

#include <qdebug>
#include <qdir>
#include <qtextstream>
#include <zel/json.h>


PersonData::PersonData(QString data_path)
    : data_path_(data_path)
    , person_data_info_(nullptr) {}

PersonData::~PersonData() {}

PersonDataInfo *PersonData::personDataInfo(QString &error) {

    person_data_info_ = new PersonDataInfo();

    QDir data_dir(data_path_);
    auto data_dirs = data_dir.entryList();
    if (data_dirs.size() <= 2) {
        error = "未找到个人化数据";
        return nullptr;
    }

    person_data_info_->filename = data_dirs[2];
    QFile data_file(data_path_ + "/" + person_data_info_->filename);
    if (data_file.open(QIODevice::ReadOnly)) {
        QTextStream in(&data_file);
        person_data_info_->header = in.readLine();
        person_data_info_->data   = in.readLine();
    } else {
        error = "打开文件失败";
        return nullptr;
    }

    data_file.close();

    auto headers = person_data_info_->header.split("/");
    auto datas   = person_data_info_->data.split(" ");
    for (int i = 0; i < headers.size(); i++) {
        person_data_info_->json_data[headers[i].toStdString()] = datas[i].toStdString();
    }

    std::string pin1 = person_data_info_->json_data["PIN1"];
    std::string ki   = person_data_info_->json_data["KI"];
    std::string op   = person_data_info_->json_data["OP"].empty() ? person_data_info_->json_data["OPC"]
                                                                  : person_data_info_->json_data["OP"];

    person_data_info_->pin1 = QString::fromStdString(pin1);
    person_data_info_->ki   = QString::fromStdString(ki);
    person_data_info_->op   = QString::fromStdString(op);

    return person_data_info_;
}
