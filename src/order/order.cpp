#include "order.h"
#include "utils/utils.h"

#include <memory>
#include <zel/core.h>
#include <zel/crypto.h>

using namespace zel::utility;
using namespace zel::fs;
using namespace zel::crypto;

Order::Order(const std::string &datagram)
    : datagram_(datagram) {}

Order::~Order() {}

bool Order::preProcessing() {
    File datagram_file(datagram_);

    std::string datagram_dir_name = "";
    if (datagram_file.extension() == ".pgp") {
        auto datagram_zip_path = join(datagram_file.dirPath(), datagram_file.prefix());
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
        if (!Utils::decompressionZipFile(datagram_zip_path, join(datagram_file.dirPath(), datagram_dir_name), true)) {
            log_error("Failed to unzip the packet: %s", datagram_zip_path.c_str());
            return false;
        }

    } else if (datagram_file.extension() == ".zip") {
        datagram_dir_name = datagram_file.prefix();

        // 解压数据包
        if (!Utils::decompressionZipFile(datagram_file.path(), join(datagram_file.dirPath(), datagram_dir_name), false)) {
            log_debug("datagram_file: %s, datagram_dir_name: %s", datagram_file.path().c_str(), datagram_dir_name.c_str());
            log_error("Failed to unzip the packet: %s", datagram_file.path().c_str());
            return false;
        }
    } else {
        datagram_dir_name = datagram_file.name();
    }

    datagram_ = join(datagram_file.dirPath(), datagram_dir_name);

    return true;
}

bool Order::processing() {

    auto walkFunc = [=](const Entry &entry) -> bool {
        if (entry.isFile()) {
            auto file = entry.file();
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
            if (person_data_info_ == nullptr) {
                if (file.name().find("PostPersoData_") == std::string::npos && file.extension() == ".prd" && file.dirPath().find("DATA") == std::string::npos) {
                    // 获取首条个人化数据
                    PersonData person_data(file.path());
                    person_data_info_ = person_data.personDataInfo();
                    if (person_data_info_ == nullptr) {
                        log_error("Failed to get person data infomation.");
                        return false;
                    }
                }
            }
        }

        if (entry.isDir()) {
            auto dir = entry.dir();
            // 解析项目脚本包
            if (dir.name().find("RD_") != std::string::npos || dir.name().find("BT_") != std::string::npos) {
                // 获取脚本信息
                Script script(dir.path());
                script_info_ = script.scriptInfo();
                if (script_info_ == nullptr) {
                    log_error("Failed to get script infomation.");
                    return false;
                }

                // 生成自动化预个人化脚本
                if (!script.autoPersonScript()) {
                    log_error("Failed to generate auto person script.");
                    return false;
                }

                // 生成自动化后个人化脚本
                if (!script.autoPostPersonScript()) {
                    log_error("Failed to generate auto post person script.");
                    return false;
                }
            }
        }

        return true;
    };

    walk(datagram_, walkFunc, true);
    return true;
}

std::shared_ptr<OrderInfo> Order::orderInfo() { return order_info_; }

std::shared_ptr<PersonDataInfo> Order::personDataInfo() { return person_data_info_; }

std::shared_ptr<ScriptInfo> Order::scriptInfo() { return script_info_; }