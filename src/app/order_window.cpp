#include "order_window.h"

#include <memory>
#include <qmessagebox>
#include <zel/core.h>

using namespace zel::utility;
using namespace zel::file_system;

OrderWindow::OrderWindow(std::vector<std::string> &datagram_format, QMainWindow *parent)
    : QMainWindow(parent)
    , datagram_format_(datagram_format)
    , ui_(std::make_shared<Ui_OrderWindow>()) {
    ui_->setupUi(this);

    init_window();

    init_ui();

    init_signal_slot();
}

OrderWindow::~OrderWindow() {}

void OrderWindow::confirmBtnClicked() {
    std::string confirm_project_number = ui_->project_number_line->text().toStdString();
    std::string confirm_order_number   = ui_->order_number_line->text().toStdString();
    if (confirm_project_number == "" || confirm_order_number == "") {
        QMessageBox::critical(this, "警告", "工程单号或订单号不能为空");
    }

    datagram_format_[0]            = confirm_project_number;
    datagram_format_[1]            = confirm_order_number;
    auto confirm_datagram_filename = String::join(datagram_format_, " ");

    std::vector<std::string> extensions                = {".zip.pgp", ".zip"};
    std::string              confirm_datagram_dir_name = confirm_datagram_filename;

    for (const std::string &ext : extensions) {
        std::size_t pos = confirm_datagram_filename.rfind(ext);
        if (pos != std::string::npos) {
            confirm_datagram_dir_name = confirm_datagram_filename.substr(0, pos);
            break;
        }
    }

    emit confirmOrder(confirm_datagram_dir_name);
}

void OrderWindow::cancelBtnClicked() { emit cancelOrder(); }

void OrderWindow::init_window() {
    // 设置窗口标题
    setWindowTitle("确认订单信息");
}

void OrderWindow::init_ui() {
    ui_->project_number_line->setText(QString(datagram_format_[0].c_str()));
    ui_->order_number_line->setText(QString(datagram_format_[1].c_str()));
}

void OrderWindow::init_signal_slot() {
    connect(ui_->confirm_btn, &QPushButton::clicked, this, &OrderWindow::confirmBtnClicked);
    connect(ui_->cancel_btn, &QPushButton::clicked, this, &OrderWindow::cancelBtnClicked);
}