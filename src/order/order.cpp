#include "order.h"
#include "utils/utils.h"

#include <vector>
#include <zel/filesystem/filepath.h>
#include <zel/utility/logger.h>
#include <zel/zel.h>
#include <memory>

using namespace zel::utility;
using namespace zel::filesystem;
using namespace zel::crypto;

Order::Order(std::shared_ptr<Path> path)
    : path_(path)
    , outgoing_(false) {}

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

        outgoing_ = true;
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

bool Order::modify() {

    // 需要替换的信息
    std::string src_order_number    = String::split(FilePath::base(path_->datagram_order), " ")[0];
    std::string src_project_number  = String::split(FilePath::base(path_->datagram_order), " ")[1];
    std::string dest_order_number   = String::split(FilePath::base(path_->order), " ")[0];
    std::string dest_project_number = String::split(FilePath::base(path_->order), " ")[1];

    Directory order_dir(path_->order);
    if (!order_dir.exists()) {
        order_dir.create();
    }

    auto walkFunc = [=](std::string relative_path, Directory dir, File file) -> bool {
        if (file.exists()) {
            // 修改 xlsx 文件中的项目信息
            if (file.extension() == ".xlsx") {
                Utils::replaceStringInXlsx(file.path(), src_order_number, dest_order_number);
                Utils::replaceStringInXlsx(file.path(), src_project_number, dest_project_number);
            }

            // 修改文件名
            auto filename = String::replace(relative_path, src_order_number, dest_order_number);
            filename      = String::replace(filename, src_project_number, dest_project_number);
            if (!file.copy(FilePath::join(path_->order, filename))) {
                return false;
            }
        }

        return true;
    };

    FilePath::walk(path_->datagram_order, walkFunc, true);

    // 删除原目录
    Directory datagram_dir(path_->datagram_order);
    return datagram_dir.remove();
}

bool Order::processing() {

    // 获取订单信息
    std::shared_ptr<OrderInfo> order_info;
    if (outgoing_)
        order_info_ = orderInfoOutgoing(FilePath::base(path_->order));
    else
        order_info_ = orderInfo(FilePath::base(path_->order));

    if (order_info_ == nullptr) {
        log_error("Failed to get order infomation.");
        return false;
    }

    // 获取首条个人化数据
    PersonData person_data(path_->data);
    person_data_info_ = person_data.personDataInfo();
    if (person_data_info_ == nullptr) {
        log_error("Failed to get person data infomation.");
        return false;
    }

    if (outgoing_) {
        // 生成截图文件夹
        if (!screenshotDir()) {
            log_error("Failed to generate screenshot dir.");
            return false;
        }

        // 生成打印文件夹
        if (!printDir()) {
            log_error("Failed to generate print dir.");
            return false;
        }

        // 生成标签数据文件夹
        if (!path_->tag_data.empty()) {
            if (!tagDataDir()) {
                log_error("Failed to generate tag data dir.");
                return false;
            }
        }
    }

    // 获取脚本信息
    Script script(path_->script);
    script_info_ = script.scriptInfo();
    if (script_info_ == nullptr) {
        log_error("Failed to get script infomation.");
        return false;
    }

    return true;
}

bool Order::backup(const std::string &backup_path) {
    // 获取年和月
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm     now_tm;
    localtime_s(&now_tm, &now_time_t); // Windows 平台
    int year  = now_tm.tm_year + 1900; // tm_year 是从 1900 年起计数的
    int month = now_tm.tm_mon + 1;     // tm_mon 是从 0 开始的，所以要加 1

    // 创建目录
    std::string save_path = FilePath::join(backup_path, std::to_string(year) + " 年", std::to_string(month) + " 月");
    Directory   backup_dir(save_path);
    if (!backup_dir.exists()) {
        backup_dir.create();
    }

    // 压缩订单数据包
    return Utils::compressionZipFile(path_->order, save_path, false);
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

bool Order::screenshotDir() {

    // 创建截图文件夹
    Directory screenshot_dir(path_->screenshot);
    if (!screenshot_dir.exists()) {
        screenshot_dir.create();
    }

    return true;
};

bool Order::printDir() {

    // 创建打印文件夹
    Directory print_dir(path_->print);
    if (!print_dir.exists()) {
        print_dir.create();
    }

    // 拷贝需要打印的文件
    auto walkFunc = [=](std::string relative_path, Directory dir, File file) -> bool {
        if (file.exists()) {
            if (file.extension() == ".xlsx" || file.extension() == ".xls") {
                if (!file.copy(FilePath::join(path_->print, file.name()))) {
                    return false;
                }
            }
        }
        return true;
    };

    return FilePath::walk(path_->order, walkFunc);
}

bool Order::tagDataDir() {

    // 压缩文件
    if (!Utils::compressionZipFile(path_->tag_data)) return false;

    // 剪切标签数据压缩包
    File tag_data_file(path_->tag_data + ".zip");
    if (!tag_data_file.move(FilePath::join(path_->temp, tag_data_file.name()))) {
        return false;
    }

    return true;
}

std::shared_ptr<OrderInfo> Order::orderInfo(const std::string &order_dir_name) {
    order_info_                 = std::make_shared<OrderInfo>();
    order_info_->order_dir_name = order_dir_name;

    auto infos                  = String::split(order_dir_name, " ");
    order_info_->project_number = infos[0];

    auto project_name_match = String::matches(infos[1], "[A-Z]+[0-9]+");
    if (project_name_match.size() != 0) {
        order_info_->order_number = infos[0];
        order_info_->project_name = project_name_match[0];
    } else {
        project_name_match = String::matches(infos[2], "[A-Z]+[0-9]+");
        if (project_name_match.size() != 0) {
            order_info_->order_number = infos[1];
            order_info_->project_name = project_name_match[0];
        }
    }

    path_->order = FilePath::join(path_->directory, order_info_->order_dir_name);
    path_->data  = FilePath::join(path_->order, "INP");

    auto walkFunc = [&](std::string relative_path, Directory dir, File file) -> bool {
        if (dir.exists()) {
            if (dir.name().rfind("Tag_data") != std::string::npos) {
                path_->tag_data = dir.path();
            } else if (String::matches(dir.name(), "([A-Z0-9]+_[A-Z0-9]+_[A-Z0-9]+)").size() != 0) {
                order_info_->script_package = dir.name();
                path_->script               = FilePath::join(dir.path(), "Script");
                auto infos                  = String::split(dir.name(), "_");
                order_info_->chip_model     = String::matches(dir.name(), "_C([A-Z0-9]+)")[0];
                order_info_->rf_code        = "RF_F1_" + String::matches(dir.name(), "(P[A-Z0-9_]+)_C")[0] + " " + order_info_->chip_model;
            }
        }
        return true;
    };
    FilePath::walk(FilePath::join(path_->order, "DATA"), walkFunc);

    return order_info_;
}

std::shared_ptr<OrderInfo> Order::orderInfoOutgoing(const std::string &order_dir_name) {
    order_info_                 = std::make_shared<OrderInfo>();
    order_info_->order_dir_name = order_dir_name;

    auto infos                  = String::split(order_dir_name, " ");
    order_info_->project_number = infos[0];
    order_info_->order_number   = infos[1];
    order_info_->project_name   = String::matches(infos[2], "[A-Z]+[0-9]+")[0];

    path_->order      = FilePath::join(path_->directory, order_info_->order_dir_name);
    path_->temp       = FilePath::join(path_->order, "TEMP", order_info_->project_number);
    path_->print      = FilePath::join(path_->temp, "打印 " + order_info_->project_number + " " + order_info_->order_number + " " + order_info_->project_name);
    path_->screenshot = FilePath::join(path_->temp, "截图 " + order_info_->project_number + " " + order_info_->order_number + " " + order_info_->project_name);

    auto walkFunc = [&](std::string relative_path, Directory dir, File file) -> bool {
        if (dir.exists()) {
            if (dir.name() == "DATA") {
                path_->data = dir.path();
            } else if (dir.name().rfind("Tag_data") != std::string::npos) {
                path_->tag_data = dir.path();
            } else if (String::matches(dir.name(), "([A-Z0-9]+_[A-Z0-9]+_[A-Z0-9]+)").size() != 0) {
                order_info_->script_package = dir.name();
                path_->script               = dir.path();
                auto infos                  = String::split(dir.name(), "_");
                order_info_->chip_model     = String::matches(dir.name(), "_C([A-Z0-9]+)")[0];
                order_info_->rf_code        = "XH_RF_" + String::matches(dir.name(), "(P[A-Z0-9_]+)_C")[0] + " " + order_info_->chip_model;
            }
        }
        return true;
    };
    FilePath::walk(path_->order, walkFunc);

    return order_info_;
}
