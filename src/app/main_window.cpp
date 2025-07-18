#include "main_window.h"

#include "order_window.h"
#include "task/upload_file.hpp"
#include "task/handle_order.hpp"
#include "task/clear_card.hpp"
#include "task/write_card.hpp"
#include "clear_card_loading.h"
#include "write_card_loading.h"
#include "myorm/database.h"

#include <memory>
#include <qdesktopservices.h>
#include <qfiledialog.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <string>
#include <qclipboard>
#include <qdesktopservices>
#include <qdragenterevent>
#include <qmessagebox>
#include <qmimedata>
#include <qpushbutton>
#include <qtextstream>
#include <qfiledialog>
#include <qstringlistmodel>
#include <qcompleter>

#include <zel/utility/logger.h>
using namespace zel::utility;
using namespace zel::filesystem;

#include <xhlanguage/card-reader/card_reader_factory.hpp>
using namespace xhlanguage::reader;

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_MainWindow)
    , order_window_(nullptr)
    , path_(nullptr)
    , order_info_(nullptr)
    , person_data_info_(nullptr)
    , script_info_(nullptr)
    , card_reader_(nullptr)
    , tabulation_(nullptr)
    , db_(nullptr) {
    ui_->setupUi(this);

    // 初始化窗口
    initWindow();

    // 初始化配置
    initConfig("config.ini");

    // 初始化UI
    initUI();

    // 初始化信号和槽
    initSignalSlot();

    // 初始化日志器
    initLogger("DataHandler.log");

    // 初始化读卡器
    initCardReader();

    // 初始化数据库
    initDatabase();
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

    std::string datagram_path = urls.first().toLocalFile().toStdString();
    path_                     = std::make_shared<Path>(datagram_path);
    path_->directory          = FilePath::dir(datagram_path);

    auto datagram_format = String::split(FilePath::base(datagram_path), " ");
    if (datagram_format.size() < 6) {
        QMessageBox::critical(this, "警告", "未知文件，请拖入星汉总部数据包");
        return;
    }

    // 确认订单
    order_window_ = new OrderWindow(datagram_format, this);

    connect(order_window_, &OrderWindow::confirmOrder, this, &MainWindow::confirmOrder);
    connect(order_window_, &OrderWindow::cancelOrder, this, &MainWindow::cancelOrder);

    order_window_->show();
}

void MainWindow::saveBtnClicked() {
    std::string mysql_host          = ui_->mysql_ip_line->text().toStdString();
    int         mysql_port          = ui_->mysql_port_line->text().toInt();
    std::string mysql_username      = ui_->mysql_username_line->text().toStdString();
    std::string mysql_password      = ui_->mysql_password_line->text().toStdString();
    std::string mysql_sim_database  = ui_->sim_database_line->text().toStdString();
    std::string mysql_bank_database = ui_->bank_database_line->text().toStdString();
    std::string ftp_host            = ui_->ftp_ip_line->text().toStdString();
    int         ftp_port            = ui_->ftp_port_line->text().toInt();
    std::string ftp_username        = ui_->ftp_username_line->text().toStdString();
    std::string ftp_password        = ui_->ftp_password_line->text().toStdString();
    std::string remote_prd_path     = ui_->prd_line->text().toStdString();
    std::string remote_temp_path    = ui_->temp_line->text().toStdString();
    std::string local_backup_path   = ui_->backup_line->text().toStdString();
    std::string bank_path           = ui_->bank_path_line->text().toStdString();
    std::string order_no            = ui_->order_no_line->text().toStdString();
    std::string order_quantity      = ui_->order_quantity_line->text().toStdString();
    std::string data                = ui_->data_line->text().toStdString();

    ini_.set("mysql", "host", mysql_host);
    ini_.set("mysql", "port", mysql_port);
    ini_.set("mysql", "username", mysql_username);
    ini_.set("mysql", "password", mysql_password);
    ini_.set("mysql", "sim_database", mysql_sim_database);
    ini_.set("mysql", "bank_database", mysql_bank_database);
    ini_.set("ftp", "host", ftp_host);
    ini_.set("ftp", "port", ftp_port);
    ini_.set("ftp", "username", ftp_username);
    ini_.set("ftp", "password", ftp_password);
    ini_.set("path", "remote_prd_path", remote_prd_path);
    ini_.set("path", "remote_temp_path", remote_temp_path);
    ini_.set("path", "local_backup_path", local_backup_path);
    ini_.set("template", "bank_path", bank_path);
    ini_.set("template", "order_no", order_no);
    ini_.set("template", "order_quantity", order_quantity);
    ini_.set("template", "data", data);

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
    loading_ = new Loading(this);
    loading_->setWindowTitle("正在上传个人化数据...");
    loading_->show();

    // 将个人化数据上传到FTP服务器
    std::string remote_prd_path = ini_["path"]["remote_prd_path"].asString() + "/" + order_info_->project_number;
    std::string local_prd_path  = path_->data;
    auto        upload_file     = new UploadFile(ini_, local_prd_path, remote_prd_path, false, path_);

    // 连接信号槽
    connect(upload_file, &UploadFile::failure, this, &MainWindow::uploadFileFailure);
    connect(upload_file, &UploadFile::success, this, &MainWindow::uploadFileSuccess);

    // 启动工作线程
    upload_file->start();
}

void MainWindow::uploadTempBtnClicked() {
    // 获取截图文件数量
    Directory dir(path_->screenshot);
    int       file_count = dir.count();
    if (file_count < 5) {
        QMessageBox::StandardButton box;
        box =
            QMessageBox::question(this, "提示", "截图文件夹数量为 " + QString::number(file_count) + " 个, 是否继续上传？", QMessageBox::Yes | QMessageBox::No);
        if (box == QMessageBox::No) return;
    }

    loading_->setWindowTitle("正在上传临时文件...");
    loading_->show();

    std::string remote_temp_path = ini_["path"]["remote_temp_path"];
    std::string local_temp_path  = FilePath::dir(path_->temp);
    auto        upload_prd       = new UploadFile(ini_, local_temp_path, remote_temp_path, true, path_);

    // 连接信号槽
    connect(upload_prd, &UploadFile::failure, this, &MainWindow::uploadFileFailure);
    connect(upload_prd, &UploadFile::success, this, &MainWindow::uploadFileSuccess);

    // 启动工作线程
    upload_prd->start();
}

void MainWindow::selectGeneratePathBtnClicked() {

    QString order_no = ui_->order_combo_box->currentText();

    // 获取桌面路径
    QString desktop_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString default_path = desktop_path + "/数据分配表" + order_no + ".xlsx";
    QString file_path    = QFileDialog::getSaveFileName(this, "选择数据分配表路径", default_path, "Excel 文件 (*.xlsx)");

    if (file_path.isEmpty()) return;

    ui_->generate_path_line->setText(file_path);
}

void MainWindow::generateDistributionRecordBtnClicked() {
    auto order_number = ui_->order_combo_box->currentText().toStdString();
    auto data_field   = ui_->data_field_line->text().toStdString();

    if (order_number.empty()) {
        QMessageBox::critical(this, "错误", "请输入订单号");
        return;
    }

    if (data_field.empty()) {
        QMessageBox::critical(this, "错误", "请输入数据项");
        return;
    }

    if (!tabulation_->distributionRecord(order_number, data_field)) {
        QMessageBox::critical(this, "错误", "请核对订单号与数据项是否正确");
        return;
    }

    auto template_path = ui_->bank_path_line->text().toStdString();
    auto generate_path = ui_->generate_path_line->text().toStdString();
    tabulation_->generateDistributionRecords(template_path, generate_path);

    QMessageBox success_box(this);
    success_box.setWindowTitle("提示");
    success_box.setText(QString("生成成功"));
    QPixmap pix(":/image/success.png");
    pix = pix.scaled(32, 32);
    success_box.setIconPixmap(pix);
    success_box.setStandardButtons(QMessageBox::Ok);
    success_box.setButtonText(QMessageBox::Ok, "确定");
    success_box.exec();
}

void MainWindow::selectTemplatePathBtnClicked() {
    QString file_path = QFileDialog::getOpenFileName(this, "选择数据分配表路径", "templates", "*.xlsx");

    if (file_path.isEmpty()) return;

    ui_->bank_path_line->setText(file_path);
}

void MainWindow::confirmOrder(const std::string &confirm_datagram_dir_name) {
    order_window_->hide();

    loading_ = new Loading(this);
    loading_->setWindowTitle("订单处理中...");
    loading_->show();

    path_->order = FilePath::join(path_->directory, confirm_datagram_dir_name);

    auto handleOrder = new HandleOrder(path_);

    // 连接信号槽
    connect(handleOrder, &HandleOrder::failure, this, &MainWindow::handleOrderFailure);
    connect(handleOrder, &HandleOrder::success, this, &MainWindow::handleOrderSuccess);

    // 启动工作线程
    handleOrder->start();
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

void MainWindow::uploadFileFailure(const QString &err_type, const QString &err_msg) {
    loading_->close();
    QMessageBox::critical(this, err_type, err_msg);
}

void MainWindow::uploadFileSuccess() {
    ui_->upload_temp_btn->setDisabled(false);
    loading_->close();

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

void MainWindow::handleOrderSuccess(std::shared_ptr<OrderInfo> order_info, std::shared_ptr<PersonDataInfo> person_data_info,
                                    std::shared_ptr<ScriptInfo> script_info) {
    loading_->hide();
    order_info_       = order_info;
    person_data_info_ = person_data_info;
    script_info_      = script_info;

    // 显示订单信息
    showInfo();

    // // 显示路径
    // order_->showPath();

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

void MainWindow::handleOrderFailure(const QString &err_msg) {
    loading_->hide();
    QMessageBox::critical(this, "警告", err_msg);
}

void MainWindow::initWindow() {

    // 设置窗口标题
    setWindowTitle("智能卡生产预处理软件 v3.0.3");

    ui_->add_dir_widget->setAcceptDrops(false);
    setAcceptDrops(true);
}

void MainWindow::initUI() {

    // 使窗口始终在其他窗口之上
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    buttonDisabled(true);
    std::string mysql_host          = ini_["mysql"]["host"];
    int         mysql_port          = ini_["mysql"]["port"];
    std::string mysql_username      = ini_["mysql"]["username"];
    std::string mysql_password      = ini_["mysql"]["password"];
    std::string mysql_bank_database = ini_["mysql"]["bank_database"];
    std::string mysql_sim_database  = ini_["mysql"]["sim_database"];
    std::string ftp_host            = ini_["ftp"]["host"];
    int         ftp_port            = ini_["ftp"]["port"];
    std::string ftp_username        = ini_["ftp"]["username"];
    std::string ftp_password        = ini_["ftp"]["password"];
    std::string remote_prd_path     = ini_["path"]["remote_prd_path"];
    std::string remote_temp_path    = ini_["path"]["remote_temp_path"];
    std::string local_backup_path   = ini_["path"]["local_backup_path"];
    std::string bank_path           = ini_["template"]["bank_path"];
    std::string order_no            = ini_["template"]["order_no"];
    std::string order_quantity      = ini_["template"]["order_quantity"];
    std::string data                = ini_["template"]["data"];

    ui_->mysql_ip_line->setText(QString::fromStdString(mysql_host));
    ui_->mysql_port_line->setText(QString::number(mysql_port));
    ui_->mysql_username_line->setText(QString::fromStdString(mysql_username));
    ui_->mysql_password_line->setText(QString::fromStdString(mysql_password));
    ui_->bank_database_line->setText(QString::fromStdString(mysql_bank_database));
    ui_->sim_database_line->setText(QString::fromStdString(mysql_sim_database));
    ui_->ftp_ip_line->setText(QString::fromStdString(ftp_host));
    ui_->ftp_port_line->setText(QString::number(ftp_port));
    ui_->ftp_username_line->setText(QString::fromStdString(ftp_username));
    ui_->ftp_password_line->setText(QString::fromStdString(ftp_password));
    ui_->prd_line->setText(QString::fromStdString(remote_prd_path));
    ui_->temp_line->setText(QString::fromStdString(remote_temp_path));
    ui_->backup_line->setText(QString::fromStdString(local_backup_path));
    ui_->bank_path_line->setText(QString::fromStdString(bank_path));
    ui_->order_no_line->setText(QString::fromStdString(order_no));
    ui_->order_quantity_line->setText(QString::fromStdString(order_quantity));
    ui_->data_line->setText(QString::fromStdString(data));

    ui_->clear_card_btn->setDisabled(true);
    ui_->write_card_btn->setDisabled(true);
}

void MainWindow::initSignalSlot() {
    QClipboard *clip = QApplication::clipboard();

    // 信息 - 订单信息
    connect(ui_->project_number_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->project_number.c_str())); });
    connect(ui_->order_number_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->order_number.c_str())); });
    connect(ui_->project_name_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->project_name.c_str())); });
    connect(ui_->chip_model_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->chip_model.c_str())); });
    connect(ui_->rf_code_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->rf_code.c_str())); });
    connect(ui_->script_package_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->script_package.c_str())); });
    connect(ui_->pin1_btn, &QPushButton::clicked, [=]() { clip->setText(QString(person_data_info_->pin1.c_str())); });
    connect(ui_->op_btn, &QPushButton::clicked, [=]() { clip->setText(QString(person_data_info_->op.c_str())); });
    connect(ui_->ki_btn, &QPushButton::clicked, [=]() { clip->setText(QString(person_data_info_->ki.c_str())); });

    // 信息 - 脚本包信息
    connect(ui_->open_personal_btn, &QPushButton::clicked, this, &MainWindow::openPersonalBtnClicked);
    connect(ui_->open_postpersonal_btn, &QPushButton::clicked, this, &MainWindow::openPostPersonalBtnClicked);
    connect(ui_->open_check_btn, &QPushButton::clicked, this, &MainWindow::openCheckBtnClicked);
    connect(ui_->open_clear_btn, &QPushButton::clicked, this, &MainWindow::openClearCardBtnClicked);

    // 鉴权
    connect(ui_->write_card_btn, &QPushButton::clicked, this, &MainWindow::writeCardBtnClicked);
    connect(ui_->clear_card_btn, &QPushButton::clicked, this, &MainWindow::clearCardBtnClicked);
    connect(ui_->reset_card_btn, &QPushButton::clicked, this, &MainWindow::resetCardBtnClicked);
    connect(ui_->upload_prd_btn, &QPushButton::clicked, this, &MainWindow::uploadPrdBtnClicked);
    connect(ui_->upload_temp_btn, &QPushButton::clicked, this, &MainWindow::uploadTempBtnClicked);

    // 制表
    connect(ui_->select_generate_file_ptn, &QPushButton::clicked, this, &MainWindow::selectGeneratePathBtnClicked);
    connect(ui_->search_ptn, &QPushButton::clicked, this, &MainWindow::generateDistributionRecordBtnClicked);

    // 配置
    connect(ui_->select_template_file_ptn, &QPushButton::clicked, this, &MainWindow::selectTemplatePathBtnClicked);
    connect(ui_->save_btn, &QPushButton::clicked, this, &MainWindow::saveBtnClicked);
}

void MainWindow::initConfig(const std::string &config_file) {
    if (!ini_.exists(config_file)) {
        ini_.set("log", "level", 0);
        ini_.set("mysql", "host", "127.0.0.1");
        ini_.set("mysql", "port", "3306");
        ini_.set("mysql", "username", "admin");
        ini_.set("mysql", "password", "admin");
        ini_.set("mysql", "sim_database", "dms");
        ini_.set("mysql", "bank_database", "dms_bank");
        ini_.set("ftp", "host", "127.0.0.1");
        ini_.set("ftp", "port", "21");
        ini_.set("ftp", "username", "admin");
        ini_.set("ftp", "password", "admin");
        ini_.set("path", "remote_prd_path", "/data/ftp/output/PRD");
        ini_.set("path", "remote_temp_path", "/data/ftp/output/临时存放");
        ini_.set("path", "local_backup_path", "D:/备份");
        ini_.set("template", "bank_path", "template/银行卡模板.xlsx");
        ini_.set("tempalte", "order_no", "工程单号(Order No.):");
        ini_.set("tempalte", "order_quantity", "下单数量(Qty):");
        ini_.set("tempalte", "data", "Product File Name");

        ini_.save(config_file);
    } else {
        ini_.load(config_file);
    }
}

void MainWindow::initLogger(const std::string &log_file) {
    auto logger = Logger::instance();
    if (!logger->isOpen()) {
        logger->open(log_file);
        logger->setFormat(false);
        int level = ini_["log"]["level"];
        logger->setLevel(Logger::Level(level));
    }
}

void MainWindow::initCardReader() {
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

void MainWindow::initDatabase() {
    db_ = std::make_shared<zel::myorm::Database>();
    if (!db_->connect(ini_["mysql"]["host"], ini_["mysql"]["port"], ini_["mysql"]["username"], ini_["mysql"]["password"], ini_["mysql"]["bank_database"])) {
        log_error("Failed to connect to database");
        QMessageBox::critical(this, "警告", "连接数据库失败，请检查配置");
        ui_->search_ptn->setDisabled(true);
        return;
    }

    log_debug("Connected to database");

    tabulation_     = std::make_shared<Tabulation>(db_, ini_);
    auto order_list = tabulation_->orderList();
    reverse(order_list.begin(), order_list.end());
    QStringList items;
    for (auto order : order_list) {
        items.append(QString::fromStdString(order));
    }

    ui_->order_combo_box->addItems(items);
    ui_->order_combo_box->setCurrentText("");
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void MainWindow::buttonDisabled(bool disabled) {
    ui_->project_number_btn->setDisabled(disabled);
    ui_->order_number_btn->setDisabled(disabled);
    ui_->project_name_btn->setDisabled(disabled);
    ui_->chip_model_btn->setDisabled(disabled);
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
    ui_->upload_temp_btn->setDisabled(disabled);
}

void MainWindow::showInfo() {
    ui_->project_number_line->setText(QString(order_info_->project_number.c_str()));
    ui_->project_number_line->setCursorPosition(0);
    ui_->order_number_line->setText(QString(order_info_->order_number.c_str()));
    ui_->project_name_line->setText(QString(order_info_->project_name.c_str()));
    ui_->chip_model_line->setText(QString(order_info_->chip_model.c_str()));
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
