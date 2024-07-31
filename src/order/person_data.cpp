#include "person_data.h"

#include <zel/json.h>
using namespace zel::json;

#include <zel/filesystem.h>
using namespace zel::filesystem;

#include <zel/crypto.h>
using namespace zel::crypto;

#include "zel/utility.h"
using namespace zel::utility;

#include <memory>
#include <iostream>

PersonData::PersonData(const std::string &data_path)
    : data_path_(data_path)
    , person_data_info_(nullptr) {}

PersonData::~PersonData() {}

std::shared_ptr<PersonDataInfo> PersonData::personDataInfo() {

    person_data_info_ = std::make_shared<PersonDataInfo>();

    Directory data_dir(data_path_);
    auto      person_data_files = data_dir.files();

    auto person_data_file_path_pgp = person_data_files->at(0).path();
    int  pos                       = person_data_file_path_pgp.find_last_of(".");
    if (pos == std::string::npos) return nullptr;
    auto person_data_file_path = person_data_file_path_pgp.substr(0, pos);

    // GPG解密个人化数据文件
    try {
        Gpg gpg("libgpgme-11.dll");
        gpg.decryptFile(person_data_file_path_pgp, person_data_file_path);
    } catch (const std::exception &e) {
        log_error("%s error: %s", person_data_file_path_pgp.c_str(), e.what());
        return nullptr;
    }

    File person_data_file(person_data_file_path);

    person_data_info_->filename = person_data_file.name();
    if (!person_data_file.readLine(person_data_info_->header)) return nullptr;
    if (!person_data_file.readLine(person_data_info_->data)) return nullptr;

    person_data_info_->json_data = jsonData();

    if (!person_data_file.remove()) return nullptr;

    return person_data_info_;
}

zel::json::Json PersonData::jsonData() {

    Json json;

    auto headers = String::split(person_data_info_->header, "/");
    auto datas   = String::split(person_data_info_->data, " ");
    for (int i = 0; i < headers.size(); i++) {
        json[headers[i]] = datas[i];
    }

    person_data_info_->pin1 = json["PIN1"].asString();
    person_data_info_->ki   = json["KI"].asString();
    std::string op          = json["OP"].empty() ? json["OPC"] : json["OP"];
    person_data_info_->op   = op;

    return json;
}