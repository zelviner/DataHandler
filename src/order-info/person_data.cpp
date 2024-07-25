#include "person_data.h"

#include <zel/json.h>
using namespace zel::json;

#include <zel/filesystem.h>
using namespace zel::filesystem;

#include <zel/crypto.h>
using namespace zel::crypto;

#include <memory>
#include <iostream>

PersonData::PersonData(const std::string &data_path)
    : data_path_(data_path)
    , person_data_info_(nullptr) {}

PersonData::~PersonData() {}

std::shared_ptr<PersonDataInfo> PersonData::personDataInfo(std::string &error) {

    person_data_info_ = std::make_shared<PersonDataInfo>();

    Directory data_dir(data_path_);
    auto      personal_data_files = data_dir.files();

    auto personal_data_filename_pgp = personal_data_files->at(0).path();
    int  pos                        = personal_data_filename_pgp.find_last_of(".");
    if (pos == std::string::npos) return person_data_info_;
    auto personal_data_filename = personal_data_filename_pgp.substr(0, pos);

    // GPG解密个人化数据文件
    try {
        Gpg gpg("libgpgme-11.dll");
        gpg.decryptFile(personal_data_filename_pgp, personal_data_filename);
    } catch (const std::exception &e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    File person_data_file(personal_data_filename);

    person_data_info_->filename = person_data_file.name();

    // QDir data_dir(data_path_);
    // auto data_dirs = data_dir.entryList();
    // if (data_dirs.size() <= 2) {
    //     error = "未找到个人化数据";
    //     return nullptr;
    // }

    // person_data_info_->filename = data_dirs[2];
    // QFile data_file(data_path_ + "/" + person_data_info_->filename);
    // if (data_file.open(QIODevice::ReadOnly)) {
    //     QTextStream in(&data_file);
    //     person_data_info_->header = in.readLine();
    //     person_data_info_->data   = in.readLine();
    // } else {
    //     error = "打开文件失败";
    //     return nullptr;
    // }

    // data_file.close();

    // auto headers = person_data_info_->header.split("/");
    // auto datas   = person_data_info_->data.split(" ");
    // for (int i = 0; i < headers.size(); i++) {
    //     person_data_info_->json_data[headers[i].toStdString()] = datas[i].toStdString();
    // }

    // std::string pin1 = person_data_info_->json_data["PIN1"];
    // std::string ki   = person_data_info_->json_data["KI"];
    // std::string op   = person_data_info_->json_data["OP"].empty() ? person_data_info_->json_data["OPC"] : person_data_info_->json_data["OP"];

    // person_data_info_->pin1 = std::string::fromStdString(pin1);
    // person_data_info_->ki   = std::string::fromStdString(ki);
    // person_data_info_->op   = std::string::fromStdString(op);

    return person_data_info_;
}
