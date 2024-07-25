#include "order_window.h"
#include "task/upload_file.hpp"
#include "main_window.h"

#include "clear_card_loading.h"
#include "order-processing/order_processing.h"
#include "task/clear_card.hpp"
#include "task/write_card.hpp"
#include "write_card_loading.h"
#include "utils/utils.h"

#include <cstddef>
#include <memory>
#include <qmainwindow.h>
#include <type_traits>

#include <zel/utility.h>
using namespace zel::utility;

#include <zel/filesystem.h>
using namespace zel::filesystem;

#include <xhlanguage/card_reader.h>

#include <qclipboard>
#include <qdebug>
#include <qdesktopservices>
#include <qdir>
#include <qdragenterevent>
#include <qmessagebox>
#include <qmimedata>
#include <qpushbutton>
#include <qtextstream>
#include <tchar.h>

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_MainWindow)
    , order_window_(nullptr)
    , path_(nullptr)
    , order_info_(nullptr)
    , person_data_info_(nullptr)
    , script_info_(nullptr)
    , card_reader_(nullptr) {
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

    if (card_reader_ != nullptr) {
        card_reader_->disconnect();
        card_reader_ = nullptr;
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.empty()) return;

    path_ = std::make_shared<Path>(urls.first().toLocalFile().toStdString());

    // 确认订单
    order_window_ = new OrderWindow(path_->datagram);

    connect(order_window_, &OrderWindow::confirmOrder, this, &MainWindow::confirmOrder);
    connect(order_window_, &OrderWindow::cancelOrder, this, &MainWindow::cancelOrder);

    order_window_->show();
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
        QMessageBox::information(this, "提示", "保存成功");
    } else {
        QMessageBox::critical(this, "错误", "保存失败");
    }
}

void MainWindow::openPersonalBtnClicked() {
    std::string path = path_->script + "/" + script_info_->person_filename;
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString(path.c_str())));
}

void MainWindow::openPostPersonalBtnClicked() {
    std::string path = path_->script + "/" + script_info_->post_person_filename;
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString(path.c_str())));
}

void MainWindow::openCheckBtnClicked() {
    std::string path = path_->script + "/" + script_info_->check_filename;
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString(path.c_str())));
}

void MainWindow::openClearCardBtnClicked() {
    std::string path = path_->script + "/" + script_info_->clear_filename;
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString(path.c_str())));
}

void MainWindow::resetCardBtnClicked() {

    try {
        // 连接读卡器
        card_reader_->connect(ui_->reader_combo_box->currentText().toStdString());
    } catch (std::exception &e) {
        QString err_msg = "connect card reader error: " + QString::fromStdString(e.what());
        QMessageBox::critical(this, "错误", err_msg);
        return;
    }

    // 复位卡片
    try {
        auto atr = card_reader_->reset();
        ui_->current_card_line->setText(QString::fromStdString(atr));
        card_reader_->disconnect();
    } catch (std::exception &e) {
        QString err_msg = "reset card error: " + QString::fromStdString(e.what());
        QMessageBox::critical(this, "错误", err_msg);
        return;
    }
}

void MainWindow::writeCardBtnClicked() {

    // 弹出写卡加载窗口, 并停留
    WriteCardLoading *write_card_loading = new WriteCardLoading(this);
    write_card_loading->show();

    // 创建工作线程
    auto write_card = new WriteCard();
    write_card->cardReader(card_reader_);
    write_card->scriptInfo(script_info_);
    write_card->jsonData(person_data_info_->json_data);
    write_card->readerName(ui_->reader_combo_box->currentText().toStdString());
    write_card->xhlanguageType(ui_->xhlanguage_combo_box->currentIndex());

    // 连接信号槽
    connect(write_card, &WriteCard::failure, write_card_loading, &WriteCardLoading::failure);
    connect(write_card, &WriteCard::success, write_card_loading, &WriteCardLoading::success);
    connect(write_card_loading, &WriteCardLoading::bareAtr, this, &MainWindow::bareAtr);
    connect(write_card_loading, &WriteCardLoading::whiteAtr, this, &MainWindow::whiteAtr);
    connect(write_card_loading, &WriteCardLoading::finishedAtr, this, &MainWindow::finishedAtr);

    // 启动工作线程
    write_card->start();
}

void MainWindow::clearCardBtnClicked() {

    // 弹出清卡加载窗口, 并停留
    ClearCardLoading *clear_card_loading = new ClearCardLoading(this);
    clear_card_loading->show();

    // 获取脚本信息
    std::string script_path = path_->directory + "/鉴权/" + order_info_->script_package;
    Script      script(script_path);
    std::string error;
    script_info_ = script.scriptInfo();
    if (script_info_ != nullptr) return;

    // 创建工作线程
    auto clear_card = new ClearCard();
    clear_card->cardReader(card_reader_);
    clear_card->scriptInfo(script_info_);
    clear_card->jsonData(person_data_info_->json_data);
    clear_card->readerName(ui_->reader_combo_box->currentText().toStdString());
    clear_card->xhlanguageType(ui_->xhlanguage_combo_box->currentIndex());

    // 连接信号槽
    connect(clear_card, &ClearCard::failure, clear_card_loading, &ClearCardLoading::failure);
    connect(clear_card, &ClearCard::success, clear_card_loading, &ClearCardLoading::success);

    // 启动工作线程
    clear_card->start();

    ui_->bare_card_line->setText("");
    ui_->white_card_line->setText("");
    ui_->finished_card_line->setText("");
}

void MainWindow::uploadPrdBtnClicked() {
    ftp_loading_ = new FtpLoading(this);
    ftp_loading_->show();

    // 将个人化数据上传到FTP服务器
    std::string remote_prd_path = ini_["ftp"]["remote_prd_path"];
    remote_prd_path += "/" + order_info_->order_number;
    // std::string local_prd_path = String::wstring2String(path_->zhDataPath().toStdWString());
    std::string local_prd_path = path_->data;
    auto        upload_file    = new UploadFile();
    upload_file->ini(ini_);
    upload_file->localFilePath(local_prd_path);
    upload_file->remoteFilePath(remote_prd_path);

    // 连接信号槽
    connect(upload_file, &UploadFile::failure, this, &MainWindow::failure);
    connect(upload_file, &UploadFile::success, this, &MainWindow::success);

    // 启动工作线程
    upload_file->start();
}

void MainWindow::uploadTempBtnClicked() {

    // 压缩截图文件夹
    QDir dir(QString(path_->screenshot.c_str()));
    int  file_count = dir.count() - 2;
    if (file_count < 6) {
        QMessageBox::StandardButton box;
        box = QMessageBox::question(this, "提示", "截图文件夹数量为 " + QString::number(file_count) + " 个, 是否继续上传？", QMessageBox::Yes | QMessageBox::No);
        if (box == QMessageBox::No) return;
    }

    if (!compressionZipFile(QString(path_->screenshot.c_str()))) {
        QMessageBox::critical(this, "错误", "压缩截图文件失败");
        return;
    }

    // 删除原截图文件夹
    if (!deleteFileOrFolder(QString(path_->screenshot.c_str()))) {
        QMessageBox::critical(this, "错误", "删除截图文件失败");
        return;
    }

    // 压缩文件
    if (!compressionZipFile(QString(path_->temp.c_str()))) {
        QMessageBox::critical(this, "错误", "压缩文件失败");
        return;
    }

    // 删除原文件
    if (!deleteFileOrFolder(QString(path_->temp.c_str()))) {
        QMessageBox::critical(this, "错误", "删除文件失败");
        return;
    }

    ftp_loading_->show();

    // TODO 将临时存放数据上传到FTP服务器
    // std::string remote_temp_path = ini_["ftp"]["remote_temp_path"];
    // std::string local_temp_path  = path_->tempPath().left(path_->tempPath().lastIndexOf("/")).toStdString();
    // auto        upload_prd       = new UploadFile();
    // upload_prd->ini(ini_);
    // upload_prd->localFilePath(local_temp_path);
    // upload_prd->remoteFilePath(remote_temp_path);

    // // 连接信号槽
    // connect(upload_prd, &UploadFile::failure, this, &MainWindow::failure);
    // connect(upload_prd, &UploadFile::success, this, &MainWindow::success);

    // // 启动工作线程
    // upload_prd->start();
}

void MainWindow::initWindow() {

    // 设置窗口标题
    setWindowTitle("智能卡生产预处理软件 v2.7.3");

    ui_->add_dir_widget->setAcceptDrops(false);
    setAcceptDrops(true);

    try {
        // 创建 PCSC 读卡器工厂
        std::unique_ptr<CardReaderFactory> card_reader_factory = std::make_unique<PCSCReaderFactory>();

        // 使用工厂创建 PCSC 读卡器
        card_reader_ = card_reader_factory->createCardReader();

        auto reader_list = card_reader_->readers();

        for (auto reader : reader_list) {
            ui_->reader_combo_box->addItem(QString::fromStdString(reader));
        }

        ui_->xhlanguage_combo_box->addItem("编译器");
        ui_->xhlanguage_combo_box->addItem("解释器");

    } catch (std::exception &e) {
        QMessageBox::critical(this, "警告", "未找到读卡器，请检查读卡器是否连接");
        ui_->reset_card_btn->setDisabled(true);
    }
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

    ui_->clear_card_btn->setDisabled(true);
    ui_->write_card_btn->setDisabled(true);
}

void MainWindow::initSignalSlot() {
    QClipboard *clip = QApplication::clipboard();
    connect(ui_->dir_name_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->order_dir_name.c_str())); });
    connect(ui_->order_id_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->order_number.c_str())); });
    connect(ui_->project_name_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->program_name.c_str())); });
    connect(ui_->rf_code_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->rf_code.c_str())); });
    connect(ui_->script_package_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->script_package.c_str())); });
    connect(ui_->pin1_btn, &QPushButton::clicked, [=]() { clip->setText(QString(person_data_info_->pin1.c_str())); });
    connect(ui_->op_btn, &QPushButton::clicked, [=]() { clip->setText(QString(person_data_info_->op.c_str())); });
    connect(ui_->ki_btn, &QPushButton::clicked, [=]() { clip->setText(QString(person_data_info_->ki.c_str())); });

    connect(ui_->open_personal_btn, &QPushButton::clicked, this, &MainWindow::openPersonalBtnClicked);
    connect(ui_->open_postpersonal_btn, &QPushButton::clicked, this, &MainWindow::openPostPersonalBtnClicked);
    connect(ui_->open_check_btn, &QPushButton::clicked, this, &MainWindow::openCheckBtnClicked);
    connect(ui_->open_clear_btn, &QPushButton::clicked, this, &MainWindow::openClearCardBtnClicked);

    connect(ui_->save_btn, &QPushButton::clicked, this, &MainWindow::saveBtnClicked);
    connect(ui_->write_card_btn, &QPushButton::clicked, this, &MainWindow::writeCardBtnClicked);
    connect(ui_->clear_card_btn, &QPushButton::clicked, this, &MainWindow::clearCardBtnClicked);
    connect(ui_->reset_card_btn, &QPushButton::clicked, this, &MainWindow::resetCardBtnClicked);
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

bool MainWindow::orderInfo(const std::string &order_dir_name) {

    // 获取订单信息
    Order order(path_);
    order_info_ = order.orderInfo(order_dir_name);
    printf("%s \n", order_info_->program_name.c_str());

    // 获取首条个人化数据
    PersonData person_data(path_->data);
    person_data_info_ = person_data.personDataInfo();
    if (person_data_info_ == nullptr) return false;

    return true;
}

bool MainWindow::orderProcessing(QString &error) {
    // TODO
    // // 创建鉴权文件夹
    // if (!order_processing_->authenticationDir(person_data_info_->filename, person_data_info_->header, person_data_info_->data)) {
    //     error = "data to auth dir error";
    //     return false;
    // }

    // 生成截图文件夹
    if (!order_processing_->screenshotDir()) {
        error = "create screenshot dir error";
        return false;
    }

    // 生成打印文件夹
    if (!order_processing_->printDir()) {
        error = "create print dir error";
        return false;
    }

    // 生成标签数据文件夹
    if (!order_processing_->tagDataDir()) {
        error = "create tag data dir error";
        return false;
    }

    // 获取脚本信息
    Script script(path_->script);
    script_info_ = script.scriptInfo();
    if (script_info_ == nullptr) return false;

    return true;
}

bool MainWindow::uploadFile2FTP(const std::string &local_file_path, const std::string &remote_file_path) {
    auto                ftp_ini = ini_["ftp"];
    zel::ftp::FtpClient ftp(ftp_ini["host"], ftp_ini["username"], ftp_ini["password"], ftp_ini["port"]);
    if (!ftp.connect()) {
        QMessageBox::critical(this, "FTP连接失败", "请检查FTP服务器IP和端口是否正确");
        return false;
    }

    if (!ftp.login()) {
        QMessageBox::critical(this, "FTP连接失败", "请检查用户名和密码是否正确");
        return false;
    }

    if (!ftp.uploadFile(local_file_path, remote_file_path)) {
        QMessageBox::critical(this, "上传文件失败", "请检查远程路径是否正确");
        return false;
    }

    return true;
}

bool MainWindow::isOrder() {
    // TODO
    // if (path_->dirPath().isEmpty() || path_->dirPath().indexOf(".") != -1) return false;

    // QDir        dir(path_->dirPath());
    // QStringList files = dir.entryList();

    // if (files.indexOf("INP") == -1 || files.indexOf("DATA") == -1) return false;

    return true;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
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
    ui_->upload_prd_btn->setDisabled(disabled);
    ui_->open_personal_btn->setDisabled(disabled);
    ui_->open_postpersonal_btn->setDisabled(disabled);
    ui_->open_check_btn->setDisabled(disabled);
    ui_->open_clear_btn->setDisabled(disabled);
    ui_->write_card_btn->setDisabled(disabled);
    ui_->clear_card_btn->setDisabled(disabled);

    ui_->upload_temp_btn->setDisabled(true);
}

void MainWindow::showInfo() {
    ui_->dir_name_line->setText(QString(order_info_->order_dir_name.c_str()));
    ui_->dir_name_line->setCursorPosition(0);
    ui_->order_id_line->setText(QString(order_info_->order_number.c_str()));
    ui_->project_name_line->setText(QString(order_info_->program_name.c_str()));
    ui_->rf_code_line->setText(QString(order_info_->rf_code.c_str()));
    ui_->script_package_line->setText(QString(order_info_->script_package.c_str()));
    ui_->script_package_line->setCursorPosition(0);

    ui_->person_script_line->setText(QString(script_info_->person_filename.c_str()));
    ui_->person_script_line->setCursorPosition(0);
    ui_->post_person_script_line->setText(QString(script_info_->post_person_filename.c_str()));
    ui_->post_person_script_line->setCursorPosition(0);
    ui_->check_script_line->setText(QString(script_info_->check_filename.c_str()));
    ui_->check_script_line->setCursorPosition(0);
    ui_->clear_script_line->setText(QString(script_info_->clear_filename.c_str()));
    ui_->clear_script_line->setCursorPosition(0);

    ui_->ki_line->setText(QString(person_data_info_->ki.c_str()));
    ui_->op_line->setText(QString(person_data_info_->op.c_str()));
    ui_->pin1_line->setText(QString(person_data_info_->pin1.c_str()));

    ui_->ds_check_box->setChecked(script_info_->has_ds);
}

void MainWindow::confirmOrder(const std::string &confirm_datagram_dir) {
    order_window_->hide();

    //   next();

    // 订单预处理
    order_processing_ = std::make_unique<OrderProcessing>(path_);
    if (!order_processing_->preProcessing(confirm_datagram_dir)) {
        QMessageBox::critical(this, "错误", "订单预处理失败，请查看日志了解详细信息");
        return;
    }

    // 获取订单信息
    if (!orderInfo(confirm_datagram_dir)) {
        return;
    }

    // // 显示路径
    // path_->show();

    QString error = "";
    if (!orderProcessing(error)) {
        QMessageBox::critical(this, "错误", error);
        return;
    }

    // 显示订单信息
    showInfo();

    // 打开复制按钮
    buttonDisabled(false);

    // 弹窗提示
    QMessageBox success_box(this);
    success_box.setWindowTitle("提示");
    success_box.setText(QString("订单处理完成"));
    QPixmap pix(":/image/success.png");
    pix = pix.scaled(32, 32);
    success_box.setIconPixmap(pix);
    success_box.setStandardButtons(QMessageBox::Ok);
    success_box.setButtonText(QMessageBox::Ok, "确定");
    success_box.exec();
}

void MainWindow::cancelOrder() { order_window_->hide(); }

void MainWindow::bareAtr(const QString &bare_atr) {
    ui_->bare_card_line->setText(bare_atr);
    ui_->bare_card_line->setCursorPosition(0);
}

void MainWindow::whiteAtr(const QString &white_atr) {
    ui_->white_card_line->setText(white_atr);
    ui_->white_card_line->setCursorPosition(0);
}

void MainWindow::finishedAtr(const QString &finished_atr) {
    ui_->finished_card_line->setText(finished_atr);
    ui_->finished_card_line->setCursorPosition(0);
}

void MainWindow::failure(const QString &err_type, const QString &err_msg) {
    ftp_loading_->close();
    QMessageBox::critical(this, err_type, err_msg);
}

void MainWindow::success() {
    ui_->upload_temp_btn->setDisabled(false);
    ftp_loading_->close();

    QMessageBox success_box(this);
    success_box.setWindowTitle("提示");
    success_box.setText(QString("上传成功"));
    QPixmap pix(":/image/success.png");
    pix = pix.scaled(32, 32);
    success_box.setIconPixmap(pix);
    success_box.setStandardButtons(QMessageBox::Ok);
    success_box.setButtonText(QMessageBox::Ok, "确定");
    success_box.exec();
}