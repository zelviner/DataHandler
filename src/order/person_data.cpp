#include "person_data.h"

#include <memory>
#include <zel/crypto.h>
#include <sstream>
#include <iomanip>

using namespace zel::json;
using namespace zel::file_system;
using namespace zel::crypto;
using namespace zel::utility;

PersonData::PersonData(const std::string &data_file, const std::string &dest_path)
    : data_file_(data_file)
    , dest_path_(dest_path)
    , person_data_info_(nullptr) {}

PersonData::~PersonData() {}

std::shared_ptr<PersonDataInfo> PersonData::personDataInfo() {

    person_data_info_ = std::make_shared<PersonDataInfo>();

    File person_data_file(data_file_);

    person_data_info_->filename = person_data_file.name();
    person_data_info_->prefix   = person_data_file.prefix();
    person_data_info_->path     = person_data_file.path();
    if (!person_data_file.readLine(person_data_info_->header)) return nullptr;
    if (!person_data_file.readLine(person_data_info_->data)) return nullptr;

    person_data_info_->json_data = json_data();

    // 拆分文件
    split_file_stream(dest_path_);

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

// bool PersonData::split_file_stream(const std::string &dest_path) {

//     const size_t batch = 10000;

//     File person_data_file(data_file_);

//     std::string header;
//     if (!person_data_file.readLine(header)) return false;

//     std::string prefix = File(data_file_).prefix();

//     std::string line;
//     size_t      count      = 0;
//     size_t      file_index = 1;

//     File out_file("");

//     while (person_data_file.readLine(line)) {

//         if (count % batch == 0) {

//             // if (out_file.isOpen()) out_file.close();

//             std::ostringstream oss;
//             oss << dest_path << "/" << prefix << "_" << std::setw(4) << std::setfill('0') << file_index++ << ".prd";
//             printf("split file: %s\n", oss.str().c_str());
//             out_file = File(oss.str());

//             if (!out_file.create()) return false;

//             out_file.write(header);
//         }

//         out_file.write(line);
//         out_file.write("\n");

//         count++;
//     }

//     return true;
// }

bool PersonData::split_file_stream(const std::string &dest_path) {

    const size_t batch = 10000;

    File person_data_file(data_file_);

    std::string header;
    if (!person_data_file.readLine(header))
        return false;

    std::string prefix = File(data_file_).prefix();

    std::string line;
    size_t count = 0;
    size_t file_index = 1;

    File out_file("");

    std::string buffer;
    buffer.reserve(batch * 200);

    while (person_data_file.readLine(line)) {

        if (count % batch == 0) {

            if (!buffer.empty()) {
                out_file.write(buffer);
                buffer.clear();
            }

            std::ostringstream oss;
            oss << dest_path << "/"
                << prefix << "_"
                << std::setw(4)
                << std::setfill('0')
                << file_index++
                << ".prd";

            printf("split file: %s\n", oss.str().c_str());

            out_file = File(oss.str());

            if (!out_file.create())
                return false;

            out_file.write(header);
        }

        buffer += line;
        buffer += '\n';

        count++;

        if (buffer.size() > 1024 * 1024) {
            out_file.write(buffer);
            buffer.clear();
        }
    }

    if (!buffer.empty())
        out_file.write(buffer);

    return true;
}