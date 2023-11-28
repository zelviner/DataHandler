#include "main_window.h"
#include "do-order/do_order.h"

#include "loading.h"
#include "task/card.h"
#include "task/input_card.hpp"

#include "public/qt-utility/qt_utility.h"
using namespace zel::qtutility;

#include "public/utility/logger.h"
#include "public/utility/string.h"
using namespace zel::utility;

#include "public/ftp/ftp.h"
using namespace zel::ftp;

#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QDragEnterEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QTextStream>
#include <tchar.h>

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_MainWindow)
    , path_(nullptr)
    , order_info_(nullptr)
    , person_data_info_(nullptr)
    , script_info_(nullptr) {
    ui_->setupUi(this);

    // 初始化窗口
    initWindow();

    // 初始化配置
    initConfig();

    // 初始化UI
    initUI();

    // 初始化信号和槽
    initSignalSlot();
}

MainWindow::~MainWindow() {
    delete ui_;

    if (order_info_ != nullptr) {
        delete order_info_;
        order_info_ = nullptr;
    }

    if (path_ != nullptr) {
        delete path_;
        path_ = nullptr;
    }

    if (script_info_ != nullptr) {
        delete script_info_;
        script_info_ = nullptr;
    }
}

bool MainWindow::getInfo(QString &error) {

    // 获取订单信息
    Order order(path_);
    order_info_ = order.orderInfo(error);
    if (error != "") return false;

    // 获取首条个人化数据
    PersonData person_data(path_->dataPath());
    person_data_info_ = person_data.personDataInfo(error);
    if (error != "") return false;

    // 获取脚本信息
    Script script(path_);
    script_info_ = script.scriptInfo(error);
    if (error != "") return false;

    return true;
}

bool MainWindow::doOrder(QString &error) {
    DoOrder do_order(path_);

    // 创建鉴权文件夹
    if (!do_order.authenticationDir(person_data_info_->filename, person_data_info_->header, person_data_info_->data)) {
        error = "data to auth dir error";
        return false;
    }

    // 创建截图文件夹
    auto filename = splitFormt(order_info_->order_dir_name, " ", 1, 3) + ".txt";
    if (!do_order.screenshotDir(filename)) {
        error = "create screenshot dir error";
        return false;
    }

    // 创建打印文件夹
    if (!do_order.printDir()) {
        error = "create print dir error";
        return false;
    }

    // 创建标签数据文件夹
    if (!do_order.tagDataDir()) {
        error = "create tag data dir error";
        return false;
    }

    // 重命名文件夹
    QString current_dir = path_->dirPath().right(path_->dirPath().indexOf("/") - 1);
    if (current_dir != order_info_->order_dir_name) {
        QString new_name = path_->dirPath().left(path_->dirPath().lastIndexOf("/") + 1) + order_info_->order_dir_name;
        renameFolder(path_->dirPath(), new_name);
        path_->dirPath(new_name);
    }

    return true;
}

void MainWindow::initWindow() {

    // 设置窗口标题
    setWindowTitle("星汉数据处理程序");

    ui_->add_dir_widget->setAcceptDrops(false);
    setAcceptDrops(true);
}

void MainWindow::initUI() {
    buttonDisabled(true);
    std::string host             = ini_["ftp"]["host"];
    int         port             = ini_["ftp"]["port"];
    std::string username         = ini_["ftp"]["username"];
    std::string password         = ini_["ftp"]["password"];
    std::string remote_prd_path  = ini_["ftp"]["remote_prd_path"];
    std::string remote_temp_path = ini_["ftp"]["remote_temp_path"];
    ui_->ip_line->setText(QString::fromStdString(host));
    ui_->port_line->setText(QString::number(port));
    ui_->username_line->setText(QString::fromStdString(username));
    ui_->password_line->setText(QString::fromStdString(password));
    ui_->prd_line->setText(QString::fromStdString(remote_prd_path));
    ui_->temp_line->setText(QString::fromStdString(remote_temp_path));
}

void MainWindow::initSignalSlot() {
    QClipboard *clip = QApplication::clipboard();
    connect(ui_->dir_name_btn, &QPushButton::clicked, [=]() { clip->setText(order_info_->order_dir_name); });
    connect(ui_->order_id_btn, &QPushButton::clicked, [=]() { clip->setText(order_info_->order_id); });
    connect(ui_->project_name_btn, &QPushButton::clicked, [=]() { clip->setText(order_info_->program_name); });
    connect(ui_->rf_code_btn, &QPushButton::clicked, [=]() { clip->setText(order_info_->rf_code); });
    connect(ui_->script_package_btn, &QPushButton::clicked, [=]() { clip->setText(order_info_->script_package); });
    connect(ui_->pin1_btn, &QPushButton::clicked, [=]() { clip->setText(person_data_info_->pin1); });
    connect(ui_->op_btn, &QPushButton::clicked, [=]() { clip->setText(person_data_info_->op); });
    connect(ui_->ki_btn, &QPushButton::clicked, [=]() { clip->setText(person_data_info_->ki); });

    connect(ui_->save_btn, &QPushButton::clicked, this, &MainWindow::saveBtnClicked);
    connect(ui_->write_card_btn, &QPushButton::clicked, this, &MainWindow::writeCardBtnClicked);
    connect(ui_->clear_card_btn, &QPushButton::clicked, this, &MainWindow::clearCardBtnClicked);
    connect(ui_->upload_prd_btn, &QPushButton::clicked, this, &MainWindow::uploadPrdBtnClicked);
    connect(ui_->upload_temp_btn, &QPushButton::clicked, this, &MainWindow::uploadTempBtnClicked);
}

void MainWindow::initConfig() {
    if (!ini_.exists("config.ini")) {
        ini_.set("ftp", "host", "127.0.0.1");
        ini_.set("ftp", "port", "21");
        ini_.set("ftp", "username", "admin");
        ini_.set("ftp", "password", "admin");
        ini_.set("ftp", "remote_prd_path", "/data/ftp/output/PRD");
        ini_.set("ftp", "remote_temp_path", "/data/ftp/output/临时存放");

        ini_.save("config.ini");
    } else {
        ini_.load("config.ini");
    }
}

void MainWindow::saveBtnClicked() {
    std::string host             = ui_->ip_line->text().toStdString();
    int         port             = ui_->port_line->text().toInt();
    std::string username         = ui_->username_line->text().toStdString();
    std::string password         = ui_->password_line->text().toStdString();
    std::string remote_prd_path  = ui_->prd_line->text().toStdString();
    std::string remote_temp_path = ui_->temp_line->text().toStdString();

    ini_.set("ftp", "host", host);
    ini_.set("ftp", "port", port);
    ini_.set("ftp", "username", username);
    ini_.set("ftp", "password", password);
    ini_.set("ftp", "remote_prd_path", remote_prd_path);
    ini_.set("ftp", "remote_temp_path", remote_temp_path);

    if (ini_.save("config.ini")) {
        QMessageBox::information(NULL, "提示", "保存成功");
    } else {
        QMessageBox::critical(NULL, "错误", "保存失败");
    }
}

void MainWindow::writeCardBtnClicked() {

    // 弹出loading窗口, 并停留
    Loading *loading = new Loading(script_info_, person_data_info_->json_data, this);
    loading->show();

    loading->inputCard();
}

void MainWindow::clearCardBtnClicked() {
    Card card(script_info_, person_data_info_->json_data);

    // 连接读卡器
    if (!card.connectCard()) {
        QMessageBox::critical(NULL, "错误", "连接读卡器失败");
        return;
    }

    // 清卡
    std::string duration;
    if (!card.clearCard(duration)) {
        QMessageBox::critical(NULL, "错误", "清卡失败");
        card.disconnectCard();
        return;
    }

    // 断开读卡器
    card.disconnectCard();

    QMessageBox::information(NULL, "提示", "清卡成功");
}

void MainWindow::uploadPrdBtnClicked() {
    // 将个人化数据上传到FTP服务器
    std::string remote_prd_path = ini_["ftp"]["remote_prd_path"];
    remote_prd_path += "/" + order_info_->order_id.toStdString();
    std::string local_prd_path = String::wstring2string(path_->ftpDataPath().toStdWString());
    uploadFile2FTP(local_prd_path, remote_prd_path);

    ui_->upload_temp_btn->setDisabled(false);
}

void MainWindow::uploadTempBtnClicked() {
    // 压缩截图文件夹
    QDir dir(path_->screenshotPath());
    int  file_count = dir.count() - 2;
    if (file_count < 6) {
        QMessageBox::information(NULL, "提示", "截图文件夹数量为 " + QString::number(file_count) + " 个");
    }
    if (!compressionZipFile(path_->screenshotPath())) {
        QMessageBox::critical(NULL, "错误", "压缩截图文件失败");
        return;
    }

    // 删除原截图文件夹
    if (!deleteFileOrFolder(path_->screenshotPath())) {
        QMessageBox::critical(NULL, "错误", "删除截图文件失败");
        return;
    }

    // 压缩文件
    if (!compressionZipFile(path_->tempPath())) {
        QMessageBox::critical(NULL, "错误", "压缩文件失败");
        return;
    }

    // 删除原文件
    if (!deleteFileOrFolder(path_->tempPath())) {
        QMessageBox::critical(NULL, "错误", "删除文件失败");
        return;
    }

    // 将临时存放数据上传到FTP服务器
    std::string remote_temp_path = ini_["ftp"]["remote_temp_path"];
    std::string local_temp_path =
        String::wstring2string(path_->tempPath().left(path_->tempPath().lastIndexOf("/")).toStdWString());
    uploadFile2FTP(local_temp_path, remote_temp_path);
}

void MainWindow::uploadFile2FTP(const std::string &local_file_path, const std::string &remote_file_path) {
    std::string host     = ini_["ftp"]["host"];
    int         port     = ini_["ftp"]["port"];
    std::string username = ini_["ftp"]["username"];
    std::string password = ini_["ftp"]["password"];

    FtpClient ftp(host, username, password, port);
    if (!ftp.connect()) {
        QMessageBox::critical(NULL, "FTP连接失败", "请检查FTP服务器IP和端口是否正确");
        return;
    }

    if (!ftp.login()) {
        QMessageBox::critical(NULL, "FTP连接失败", "请检查用户名和密码是否正确");
        return;
    }

    if (!ftp.uploadFile(local_file_path, remote_file_path)) {
        QMessageBox::critical(NULL, "上传文件失败", "请检查远程路径是否正确");
        return;
    }

    QMessageBox::information(NULL, "提示", "上传成功");
}

bool MainWindow::isOrder() {
    if (path_->dirPath().isEmpty() || path_->dirPath().indexOf(".") != -1) return false;

    QDir        dir(path_->dirPath());
    QStringList files = dir.entryList();

    if (files.indexOf("INP") == -1 || files.indexOf("DATA") == -1) return false;

    return true;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.empty()) return;

    next();

    // // 初始化日志
    // common::utility::Logger::instance()->open("order_info.log");

    path_ = new Path(urls.first().toLocalFile());
    path_->dataPath("INP");
    path_->authenticationPath("鉴权");
    path_->clearCardPath("MD5清卡脚本");

    QString error = "";
    if (!getInfo(error)) {
        QMessageBox::critical(NULL, "错误", error);
        return;
    }

    // // 显示路径
    // path_->show();

    if (!doOrder(error)) {
        QMessageBox::critical(NULL, "错误", error);
        return;
    }

    // 显示订单信息
    showInfo();

    // 打开复制按钮
    buttonDisabled(false);

    // 弹窗提示
    QMessageBox success_box;
    success_box.setWindowTitle("提示");
    success_box.setText(QString("订单处理完成"));
    QPixmap pix(":/image/success.png");
    pix = pix.scaled(32, 32);
    success_box.setIconPixmap(pix);
    success_box.setStandardButtons(QMessageBox::Ok);
    success_box.setButtonText(QMessageBox::Ok, "确定");
    success_box.exec();
}

void MainWindow::buttonDisabled(bool disabled) {
    ui_->dir_name_btn->setDisabled(disabled);
    ui_->order_id_btn->setDisabled(disabled);
    ui_->project_name_btn->setDisabled(disabled);
    ui_->rf_code_btn->setDisabled(disabled);
    ui_->script_package_btn->setDisabled(disabled);
    ui_->pin1_btn->setDisabled(disabled);
    ui_->op_btn->setDisabled(disabled);
    ui_->ki_btn->setDisabled(disabled);
    ui_->write_card_btn->setDisabled(disabled);
    ui_->upload_prd_btn->setDisabled(disabled);
    ui_->clear_card_btn->setDisabled(disabled);
    ui_->upload_temp_btn->setDisabled(true);
}

void MainWindow::showInfo() {
    ui_->dir_name_line->setText(order_info_->order_dir_name);
    ui_->order_id_line->setText(order_info_->order_id);
    ui_->project_name_line->setText(order_info_->program_name);
    ui_->rf_code_line->setText(order_info_->rf_code);
    ui_->script_package_line->setText(order_info_->script_package);

    ui_->person_script_line->setText(script_info_->person_filename);
    ui_->post_person_script_line->setText(script_info_->post_person_filename);
    ui_->check_script_line->setText(script_info_->check_filename);
    ui_->clear_script_line->setText(script_info_->clear_filename);

    ui_->ki_line->setText(person_data_info_->ki);
    ui_->op_line->setText(person_data_info_->op);
    ui_->pin1_line->setText(person_data_info_->pin1);

    ui_->ds_check_box->setChecked(script_info_->has_ds);
}

void MainWindow::next() {
    if (path_ == nullptr && script_info_ == nullptr && person_data_info_ == nullptr && order_info_ == nullptr) return;

    delete path_;
    path_ = nullptr;

    delete script_info_;
    script_info_ = nullptr;

    delete person_data_info_;
    person_data_info_ = nullptr;

    delete order_info_;
    order_info_ = nullptr;
}
