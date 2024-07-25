#include "order.h"
#include "utils/utils.h"

#include <vector>
#include <zel/utility.h>
using namespace zel::utility;

#include <zel/filesystem.h>
using namespace zel::filesystem;

#include <memory>

Order::Order(std::shared_ptr<Path> path)
    : path_(path) {}

Order::~Order() {}

std::shared_ptr<OrderInfo> Order::orderInfo(const std::string &order_dir_name) {
    order_info_                 = std::make_shared<OrderInfo>();
    order_info_->order_dir_name = order_dir_name;

    auto infos                  = String::split(order_dir_name, " ");
    order_info_->order_number   = infos[0];
    order_info_->project_number = infos[1];
    order_info_->program_name   = infos[2];

    path_->order      = FilePath::join(path_->directory, order_info_->order_dir_name);
    path_->temp       = FilePath::join(path_->order, "TEMP", order_info_->order_number);
    path_->print      = FilePath::join(path_->temp, "打印 " + order_info_->order_number + " " + order_info_->project_number);
    path_->screenshot = FilePath::join(path_->temp, "截图 " + order_info_->order_number + " " + order_info_->project_number);

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
                order_info_->rf_code        = "XH_RF_" + String::matches(dir.name(), "P[A-Z0-9_]+_C[A-Z0-9]+")[0];
            }
        }
        return true;
    };

    FilePath::walk(path_->order, walkFunc);

    return order_info_;
}
