#include "order_window.h"

#include <memory>
#include <zel/utility.h>
using namespace zel::utility;

using namespace zel::filesystem;

#include <qmessagebox>

OrderWindow::OrderWindow(const std::string &datagram_path, QMainWindow *parent)
    : QMainWindow(parent)
    , datagram_file_(std::make_unique<File>(datagram_path))
    , ui_(std::make_shared<Ui_OrderWindow>()) {
    ui_->setupUi(this);

    initWindow();

    initUI();

    initSignalSlot();
}

OrderWindow::~OrderWindow() {}

void OrderWindow::confirmBtnClicked() {
    std::string confirm_project_number = ui_->project_number_line->text().toStdString();
    std::string confirm_order_number   = ui_->order_number_line->text().toStdString();
    if (confirm_project_number == "" || confirm_order_number == "") {
        QMessageBox::critical(this, "警告", "工程单号或订单号不能为空");
    }

    datagram_format_[0] = confirm_project_number;
    datagram_format_[1] = confirm_order_number;

    auto datagram_temp = String::join(datagram_format_, " ");
    int  pos           = datagram_temp.find_last_of(".zip.pgp");
    if (pos == std::string::npos) return;
    auto confirm_datagram_dir = std::string(datagram_temp.substr(0, pos - 7).c_str());

    emit confirmOrder(confirm_datagram_dir);
}

void OrderWindow::cancelBtnClicked() { emit cancelOrder(); }

void OrderWindow::initWindow() {
    // 设置窗口标题
    setWindowTitle("确认订单信息");
}

void OrderWindow::initUI() {
    datagram_format_ = String::split(datagram_file_->name(), " ");

    ui_->project_number_line->setText(QString(datagram_format_[0].c_str()));
    ui_->order_number_line->setText(QString(datagram_format_[1].c_str()));
}

void OrderWindow::initSignalSlot() {
    connect(ui_->confirm_btn, &QPushButton::clicked, this, &OrderWindow::confirmBtnClicked);
    connect(ui_->cancel_btn, &QPushButton::clicked, this, &OrderWindow::cancelBtnClicked);
}