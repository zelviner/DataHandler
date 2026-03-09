#include "person_data.h"

#include <memory>
#include <zel/crypto.h>

using namespace zel::json;
using namespace zel::file_system;
using namespace zel::crypto;
using namespace zel::utility;

PersonData::PersonData(const std::string &data_file)
    : data_file_(data_file)
    , person_data_info_(nullptr) {}

PersonData::~PersonData() {}

std::shared_ptr<PersonDataInfo> PersonData::personDataInfo() {

    person_data_info_ = std::make_shared<PersonDataInfo>();

    File person_data_file(data_file_);

    person_data_info_->filename = person_data_file.name();
    person_data_info_->path     = person_data_file.path();
    if (!person_data_file.readLine(person_data_info_->header)) return nullptr;
    if (!person_data_file.readLine(person_data_info_->data)) return nullptr;

    person_data_info_->json_data = json_data();

    return person_data_info_;
}

zel::json::Json PersonData::json_data() {

    Json json;

    auto headers = String::split(person_data_info_->header, "/");
    auto datas   = String::split(person_data_info_->data, " ");
    for (size_t i = 0; i < headers.size(); i++) {
        json[headers[i]] = datas[i];
    }

    return json;
}