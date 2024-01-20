#include "main_window.h"

#include "app/main_window.h"
#include "public/public.h"

#include <utility/logger.h>
#include <utility/string.h>
using namespace zel::utility;

#include <ftp/ftp.h>
using namespace zel::ftp;

#include "clear_card_loading.h"
#include "do-order/do_order.h"
#include "task/clear_card.hpp"
#include "task/write_card.hpp"
#include "write_card_loading.h"

#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
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

    if (person_data_info_ != nullptr) {
        delete person_data_info_;
        person_data_info_ = nullptr;
    }

    if (card_reader_ != nullptr) {
        card_reader_->disconnect();
        card_reader_ = nullptr;
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

    // 获取脚本信息
    std::string script_path = String::wstring2string(path_->scriptPath().toStdWString());
    Script      script(script_path);
    script_info_ = script.scriptInfo(error);
    if (error != "") return false;

    return true;
}

void MainWindow::initWindow() {

    // 设置窗口标题
    setWindowTitle("星汉数据处理程序");

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
    connect(ui_->dir_name_btn, &QPushButton::clicked, [=]() { clip->setText(order_info_->order_dir_name); });
    connect(ui_->order_id_btn, &QPushButton::clicked, [=]() { clip->setText(order_info_->order_id); });
    connect(ui_->project_name_btn, &QPushButton::clicked, [=]() { clip->setText(order_info_->program_name); });
    connect(ui_->rf_code_btn, &QPushButton::clicked, [=]() { clip->setText(order_info_->rf_code); });
    connect(ui_->script_package_btn, &QPushButton::clicked, [=]() { clip->setText(order_info_->script_package); });
    connect(ui_->pin1_btn, &QPushButton::clicked, [=]() { clip->setText(person_data_info_->pin1); });
    connect(ui_->op_btn, &QPushButton::clicked, [=]() { clip->setText(person_data_info_->op); });
    connect(ui_->ki_btn, &QPushButton::clicked, [=]() { clip->setText(person_data_info_->ki); });

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
    QString path = path_->scriptPath() + "/" + script_info_->person_filename;
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::openPostPersonalBtnClicked() {
    QString path = path_->scriptPath() + "/" + script_info_->post_person_filename;
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::openCheckBtnClicked() {
    QString path = path_->scriptPath() + "/" + script_info_->check_filename;
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::openClearCardBtnClicked() {
    QString path = path_->scriptPath() + "/" + script_info_->clear_filename;
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
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
    std::string script_path = String::wstring2string(path_->dirPath().toStdWString() + L"/鉴权/" +
                                                     order_info_->script_package.toStdWString());
    Script      script(script_path);
    QString     error;
    script_info_ = script.scriptInfo(error);
    if (error != "") return;

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
    // 将个人化数据上传到FTP服务器
    std::string remote_prd_path = ini_["ftp"]["remote_prd_path"];
    remote_prd_path += "/" + order_info_->order_id.toStdString();
    std::string local_prd_path = String::wstring2string(path_->zhDataPath().toStdWString());
    uploadFile2FTP(local_prd_path, remote_prd_path);

    ui_->upload_temp_btn->setDisabled(false);
}

void MainWindow::uploadTempBtnClicked() {
    // 压缩截图文件夹
    QDir dir(path_->screenshotPath());
    int  file_count = dir.count() - 2;
    if (file_count < 6) {
        QMessageBox::information(this, "提示", "截图文件夹数量为 " + QString::number(file_count) + " 个");
    }
    if (!compressionZipFile(path_->screenshotPath())) {
        QMessageBox::critical(this, "错误", "压缩截图文件失败");
        return;
    }

    // 删除原截图文件夹
    if (!deleteFileOrFolder(path_->screenshotPath())) {
        QMessageBox::critical(this, "错误", "删除截图文件失败");
        return;
    }

    // 压缩文件
    if (!compressionZipFile(path_->tempPath())) {
        QMessageBox::critical(this, "错误", "压缩文件失败");
        return;
    }

    // 删除原文件
    if (!deleteFileOrFolder(path_->tempPath())) {
        QMessageBox::critical(this, "错误", "删除文件失败");
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
        QMessageBox::critical(this, "FTP连接失败", "请检查FTP服务器IP和端口是否正确");
        return;
    }

    if (!ftp.login()) {
        QMessageBox::critical(this, "FTP连接失败", "请检查用户名和密码是否正确");
        return;
    }

    if (!ftp.uploadFile(local_file_path, remote_file_path)) {
        QMessageBox::critical(this, "上传文件失败", "请检查远程路径是否正确");
        return;
    }

    QMessageBox::information(this, "提示", "上传成功");
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

    path_ = new Path(urls.first().toLocalFile());
    path_->dataPath("INP");
    path_->authenticationPath("鉴权");
    path_->clearCardPath("MD5清卡脚本");

    QString error = "";
    if (!getInfo(error)) {
        QMessageBox::critical(this, "错误", error);
        return;
    }

    // // 显示路径
    // path_->show();

    if (!doOrder(error)) {
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
    ui_->dir_name_line->setText(order_info_->order_dir_name);
    ui_->dir_name_line->setCursorPosition(0);
    ui_->order_id_line->setText(order_info_->order_id);
    ui_->project_name_line->setText(order_info_->program_name);
    ui_->rf_code_line->setText(order_info_->rf_code);
    ui_->script_package_line->setText(order_info_->script_package);
    ui_->script_package_line->setCursorPosition(0);

    ui_->person_script_line->setText(script_info_->person_filename);
    ui_->person_script_line->setCursorPosition(0);
    ui_->post_person_script_line->setText(script_info_->post_person_filename);
    ui_->post_person_script_line->setCursorPosition(0);
    ui_->check_script_line->setText(script_info_->check_filename);
    ui_->check_script_line->setCursorPosition(0);
    ui_->clear_script_line->setText(script_info_->clear_filename);
    ui_->clear_script_line->setCursorPosition(0);

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