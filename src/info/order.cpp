#include "order.h"
#include "public/qt-utility/qt_utility.h"
using namespace zel::qtutility;

#include <QDebug>
#include <QDir>
#include <QRegExp>
#include <QStringList>

#include <utility/logger.h>

Order::Order(Path *path)
    : path_(path)
    , order_info_(nullptr) {}

Order::~Order() {}

OrderInfo *Order::orderInfo(QString &error) {

    order_info_ = new OrderInfo();

    QDir    dir(path_->dirPath() + "/DATA");
    auto    dir_names = dir.entryList();
    QString dir_name  = "";
    for (auto name : dir_names) {
        if (name.indexOf(" ") != -1) {
            dir_name = name;
        }
    }

    if (dir_name == "") {
        error = "未找到总部订单文件夹";
        log_error(error.toStdString().c_str());
        return nullptr;
    }

    path_->orderPath("DATA/" + dir_name);

    int index = 0;
    if (dir_name.indexOf("更新") != -1) index++;

    // 获取订单号、项目名称
    QStringList info          = dir_name.split(" ");
    order_info_->order_id     = info[0 + index];
    order_info_->program_name = info[2 + index];
    QRegExp rx("[A-Z]+[0-9]+");
    if (rx.indexIn(order_info_->program_name) != -1) {
        order_info_->program_name = rx.cap(0);
    }

    order_info_->order_dir_name = splitFormt(dir_name, " ", 1 + index);

    auto temp = "临时存放/" + order_info_->order_id;
    path_->tempPath(temp);
    path_->screenshotPath(temp + "/截图 " + splitFormt(dir_name, " ", 1 + index, 3 + index));
    path_->printPath(temp + "/打印 " + splitFormt(dir_name, " ", 1 + index, 3 + index));

    dir.setPath(path_->dirPath() + "/DATA/" + dir_name);
    dir_names = dir.entryList();
    auto order_path      = "DATA/" + dir_name;
    QStringList temp_list;
    for (auto dir_name : dir_names) {
        if (dir_name.indexOf(".") == -1) {
            if (dir_name.indexOf("Tag_data") != -1) {
                path_->srcTagDataPath(order_path + "/" + dir_name);
                path_->destTagDataPath(temp + "/" + dir_name);
            } else if (dir_name.indexOf("_") != -1) {
                order_info_->script_package = dir_name;
                order_info_->rf_code        = "XH_RF_" + splitFormt(dir_name, "_", 2, 4);
                path_->scriptPath(order_path + "/" + dir_name);
            } else if (dir_name == "DATA") {
                path_->ftpDataPath(order_path + "/" + dir_name);
            } else {
                continue;
            }
        } else if (dir_name.indexOf(".xlsx") != -1 || dir_name.indexOf(".xls") != -1) {
            temp_list.push_back(order_path + "/" + dir_name);
        }
    }
    path_->printsPath(temp_list);

    return order_info_;
}
