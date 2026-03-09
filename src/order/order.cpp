#include "order.h"
#include "utils/utils.h"

#include <memory>
#include <zel/core.h>
#include <zel/crypto.h>

using namespace zel::utility;
using namespace zel::file_system;
using namespace zel::crypto;

Order::Order(std::shared_ptr<Path> path)
    : path_(path) {}

Order::~Order() {}

bool Order::preProcessing() {
    File datagram_file(path_->datagram);

    std::string datagram_dir_name = "";
    if (datagram_file.extension() == ".pgp") {
        auto datagram_zip_path = FilePath::join(datagram_file.dirPath(), datagram_file.prefix());
        datagram_dir_name      = datagram_file.prefix();
        int pos                = datagram_dir_name.rfind(".zip");
        if (pos == int(std::string::npos)) return false;
        datagram_dir_name = datagram_dir_name.substr(0, pos);

        // GPG解密数据包
        try {
            Gpg gpg("libgpgme-11.dll");
            gpg.decryptFile(datagram_file.path(), datagram_zip_path);
        } catch (const std::exception &e) {
            log_error("%s error: %s", datagram_file.path().c_str(), e.what());
            return false;
        }

        // 解压数据包
        if (!Utils::decompressionZipFile(datagram_zip_path, FilePath::join(datagram_file.dirPath(), datagram_dir_name), true)) {
            log_error("Failed to unzip the packet: %s", datagram_zip_path.c_str());
            return false;
        }

    } else if (datagram_file.extension() == ".zip") {
        datagram_dir_name = datagram_file.prefix();

        // 解压数据包
        if (!Utils::decompressionZipFile(datagram_file.path(), FilePath::join(datagram_file.dirPath(), datagram_dir_name), false)) {
            log_debug("datagram_file: %s, datagram_dir_name: %s", datagram_file.path().c_str(), datagram_dir_name.c_str());
            log_error("Failed to unzip the packet: %s", datagram_file.path().c_str());
            return false;
        }
    } else {
        datagram_dir_name = datagram_file.name();
    }

    path_->datagram_order = FilePath::join(path_->directory, datagram_dir_name);

    return true;
}

bool Order::processing() {

    auto walkFunc = [=](std::string relative_path, Directory dir, File file) -> bool {
        if (file.exists()) {
            // 解析项目信息表
            if (file.name().find("项目信息表") != std::string::npos && file.extension() == ".pdf") {
                OrderParser order_parser;
                order_info_ = order_parser.parse(file.path());
                if (order_info_ == nullptr) {
                    log_error("Failed to get order infomation.");
                    return false;
                }
            }

            // 解析个人化数据
            // if (person_data_info_ == nullptr) {
            if (file.name().find("PostPersoData_") == std::string::npos && file.extension() == ".prd") {
                // 跳过分割后的个人化数据文件
                if (file.path().find("SPLITED_INP") == std::string::npos && file.path().find("INP1") == std::string::npos) {
                    // 获取首条个人化数据
                    PersonData person_data(file.path(), path_->datagram_order + "/SPLITED_INP");
                    person_data_info_ = person_data.personDataInfo();
                    if (person_data_info_ == nullptr) {
                        log_error("Failed to get person data infomation.");
                        return false;
                    }
                }
            }
            // }
        }

        if (dir.exists()) {
            // 解析项目脚本包
            if (dir.name() == "Script") {
                // 获取脚本信息
                Script script(dir.path());
                script_info_ = script.scriptInfo();
                if (script_info_ == nullptr) {
                    log_error("Failed to get script infomation.");
                    return false;
                }
            }
        }

        return true;
    };

    return FilePath::walk(path_->datagram_order, walkFunc, true);
}

void Order::showPath() {
    printf("datagram_path: %s\n", path_->datagram.c_str());
    printf("datagram_order_path: %s\n", path_->datagram_order.c_str());
    printf("directory_path: %s\n", path_->directory.c_str());
    printf("data_path: %s\n", path_->data.c_str());
    printf("temp_path: %s\n", path_->temp.c_str());
    printf("screenshot_path: %s\n", path_->screenshot.c_str());
    printf("print_path: %s\n", path_->print.c_str());
    printf("tag_data_path: %s\n", path_->tag_data.c_str());
    printf("script_path: %s\n", path_->script.c_str());
}

std::shared_ptr<OrderInfo> Order::orderInfo() { return order_info_; }

std::shared_ptr<PersonDataInfo> Order::personDataInfo() { return person_data_info_; }

std::shared_ptr<ScriptInfo> Order::scriptInfo() { return script_info_; }