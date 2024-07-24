#include "order.h"
#include "utils/utils.h"

#include <memory>
#include <qdebug>
#include <qdir>
#include <qregexp>
#include <qstringlist>
#include <zel/utility.h>

Order::Order(std::shared_ptr<Path> path)
    : path_(path) {}

Order::~Order() {}

std::shared_ptr<OrderInfo> Order::orderInfo(QString &error) {
    order_info_ = std::make_shared<OrderInfo>();
    QDir dir(path_->dirPath());
    auto dir_names = dir.entryList();

    path_->zhOrderPath(path_->dirPath());

    auto temp = "临时存放/" + order_info_->order_number;
    path_->tempPath(temp);
    path_->screenshotPath(temp + "/截图 " + order_info_->project_number + " " + order_info_->order_number);
    path_->printPath(temp + "/打印 " + order_info_->project_number + " " + order_info_->order_number);

    dir_names = dir.entryList();
    QStringList temp_list;
    for (auto dir_name : dir_names) {
        if (dir_name.indexOf(".") == -1) {
            if (dir_name.indexOf("Tag_data") != -1) {
                // 标签数据
                path_->zhTagDataPath(path_->dirPath() + "/" + dir_name);
                path_->tagDataPath(temp + "/" + dir_name);
            } else if (dir_name.indexOf("_") != -1) {
                // 脚本包
                order_info_->script_package = dir_name;
                QString card_type           = dir_name.mid(dir_name.indexOf("_C") + 2, 4);
                order_info_->rf_code        = "XH_RF_" + splitFormt(dir_name, "_", 'P', 'C') + " " + card_type;
                path_->zhScriptPath(path_->dirPath() + "/" + dir_name);
                path_->scriptPath("鉴权/" + dir_name);
                std::string a;
            } else if (dir_name == "DATA") {
                // 个人化数据
                path_->zhDataPath(path_->dirPath() + "/" + dir_name);
            } else {
                continue;
            }
        } else if (dir_name.indexOf(".xlsx") != -1 || dir_name.indexOf(".xls") != -1) {
            temp_list.push_back(path_->dirPath() + "/" + dir_name);
        }
    }
    path_->zhPrintPaths(temp_list);
}
