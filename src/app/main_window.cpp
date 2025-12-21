#include "main_window.h"

#include "order_window.h"
#include "tabulation/tabulation.h"
#include "task/upload_file.hpp"
#include "task/handle_order.hpp"
#include "task/reset_card.hpp"
#include "task/clear_card.hpp"
#include "task/write_card.hpp"
#include "task/aka_auth.hpp"
#include "task/generating_records.hpp"
#include "clear_card_loading.h"
#include "write_card_loading.h"
#include "aka_auth_loading.h"
#include "dms/dms.h"

#include <memory>
#include <qaction.h>
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
#include <zel/core.h>
#include <zel/myorm.h>

using namespace zel::utility;
using namespace zel::file_system;

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_MainWindow)
    , order_window_(nullptr)
    , path_(nullptr)
    , order_info_(nullptr)
    , person_data_info_(nullptr)
    , script_info_(nullptr)
    , data_handler_(nullptr) {
    ui_->setupUi(this);

    // 初始化窗口
    init_window();

    // 初始化配置
    init_config("config.ini");

    // 初始化UI
    init_ui();

    // 初始化信号和槽
    init_signal_slot();

    // 初始化日志器
    init_logger("DataHandler.log");

    // 初始化读卡器
    init_card_reader();

    // 初始化数据库
    init_database();

    // 初始化鉴权脚本
    init_auth_script("auth.script");
}

MainWindow::~MainWindow() { delete ui_; }

void MainWindow::chineseLanguageAction() { switch_language("zh_CN"); }

void MainWindow::englishLanguageAction() { switch_language("en_US"); }

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.empty()) return;

    std::string datagram_path = urls.first().toLocalFile().toStdString();
    path_                     = std::make_shared<Path>(datagram_path);
    path_->directory          = FilePath::dir(datagram_path);

    auto datagram_format = String::split(FilePath::base(datagram_path), " ");
    if (datagram_format.size() < 4) {
        QMessageBox::critical(this, "警告", "未知文件，请拖入星汉总部数据包");
        return;
    }

    // 确认订单
    order_window_ = new OrderWindow(datagram_format, this);

    connect(order_window_, &OrderWindow::confirmOrder, this, &MainWindow::confirmOrder);
    connect(order_window_, &OrderWindow::cancelOrder, this, &MainWindow::cancelOrder);

    order_window_->show();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
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
    ui_->current_card_line->setText(tr("正在复位..."));

    auto reset_card = new ResetCard(ui_->reader_combo_box->currentIndex(), data_handler_);

    connect(reset_card, &ResetCard::resetSuccess, this, &MainWindow::resetCardSuccess);
    connect(reset_card, &ResetCard::resetFailure, this, &MainWindow::resetCardFailure);

    reset_card->start();
}

void MainWindow::writeCardBtnClicked() {
    // 弹出写卡加载窗口, 并停留
    WriteCardLoading *write_card_loading = new WriteCardLoading(this);
    write_card_loading->show();

    // 创建工作线程
    auto write_card = new WriteCard(script_info_, person_data_info_, ui_->reader_combo_box->currentIndex(), data_handler_);

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

    bool script_convert = ui_->script_convert_check_box->isChecked();

    // 创建工作线程
    auto clear_card = new ClearCard(script_info_, person_data_info_, ui_->reader_combo_box->currentIndex(), data_handler_, script_convert);
    // 连接信号槽
    connect(clear_card, &ClearCard::failure, clear_card_loading, &ClearCardLoading::failure);
    connect(clear_card, &ClearCard::success, clear_card_loading, &ClearCardLoading::success);

    // 启动工作线程
    clear_card->start();

    ui_->bare_card_line->setText("");
    ui_->white_card_line->setText("");
    ui_->finished_card_line->setText("");
}

void MainWindow::openAuthScriptBtnClicked() { QDesktopServices::openUrl(QUrl::fromLocalFile(QString("./auth.script"))); }

void MainWindow::akaAuthBtnClicked() {
    // 弹出清卡加载窗口, 并停留
    AkaAuthLoading *aka_auth_loading = new AkaAuthLoading(this);
    aka_auth_loading->show();

    // 创建工作线程
    auto aka_auth = new AkaAuth(script_info_, person_data_info_, ui_->reader_combo_box->currentIndex(), data_handler_, false);
    // 连接信号槽
    connect(aka_auth, &AkaAuth::failure, aka_auth_loading, &AkaAuthLoading::failure);
    connect(aka_auth, &AkaAuth::success, aka_auth_loading, &AkaAuthLoading::success);

    // 启动工作线程
    aka_auth->start();
}

void MainWindow::uploadPrdBtnClicked() {
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

void MainWindow::selectFinanceGeneratePathBtnClicked() {
    QString order_no = ui_->finance_order_combo_box->currentText();

    // 获取桌面路径
    QString desktop_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString default_path = desktop_path + "/数据分配表" + order_no + ".xlsx";
    QString file_path    = QFileDialog::getSaveFileName(this, "选择数据分配表路径", default_path, "Excel 文件 (*.xlsx)");

    if (file_path.isEmpty()) return;

    ui_->finance_generate_path_line->setText(file_path);
}

void MainWindow::generatingFinanceRecordBtnClicked() {
    // 校验配置
    if (ui_->finance_path_line->text().isEmpty() || ui_->order_no_line->text().isEmpty() || ui_->order_quantity_line->text().isEmpty() ||
        ui_->data_line->text().isEmpty()) {
        QMessageBox::critical(this, "错误", "请先配置金融模板文件路径、订单号、订单数量、数据项单元格内容");
        return;
    }

    auto order_number  = ui_->finance_order_combo_box->currentText().toStdString();
    auto data_field    = ui_->finance_data_field_line->text().toStdString();
    auto generate_path = ui_->finance_generate_path_line->text().toStdString();

    if (order_number.empty()) {
        QMessageBox::critical(this, "错误", "请输入订单号");
        return;
    }

    if (data_field.empty()) {
        QMessageBox::critical(this, "错误", "请输入数据项");
        return;
    }

    if (generate_path.empty()) {
        QMessageBox::critical(this, "错误", "请选择数据分配表路径");
        return;
    }

    loading_->setWindowTitle("金融数据分配表生成中...");
    loading_->show();

    auto template_path      = ui_->finance_path_line->text().toStdString();
    auto generating_records = new GeneratingRecords(false, finance_db_, telecom_db_, ini_, order_number, data_field, template_path, generate_path);

    // 连接信号槽
    connect(generating_records, &GeneratingRecords::failure, this, &MainWindow::generatingRecordFailure);
    connect(generating_records, &GeneratingRecords::success, this, &MainWindow::generatingRecordSuccess);

    // 启动工作线程
    generating_records->start();
}

void MainWindow::deleteTelecomOrderBtnClicked() {
    auto order_no = ui_->telecom_order_combo_box->currentText().toStdString();

    if (order_no.empty()) {
        QMessageBox::critical(this, "错误", "请先选择要删除的订单号");
        return;
    }

    authenticator_ = new Authenticator(this);
    authenticator_->show();

    connect(authenticator_, &Authenticator::confirmDeleteOrder, this, &MainWindow::confirmDeleteOrder);
    connect(authenticator_, &Authenticator::cancelDeleteOrder, this, &MainWindow::cancelDeleteOrder);
}

void MainWindow::selectTelecomGeneratePathBtnClicked() {
    QString order_no = ui_->telecom_order_combo_box->currentText();

    // 获取桌面路径
    QString desktop_path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString default_path = desktop_path + "/" + order_no + "-数据分配表" + ".xlsx";
    QString file_path    = QFileDialog::getSaveFileName(this, "选择数据分配表路径", default_path, "Excel 文件 (*.xlsx)");

    if (file_path.isEmpty()) return;

    ui_->telecom_generate_path_line->setText(file_path);
}

void MainWindow::generatingTelecomRecordBtnClicked() {
    // 校验配置
    if (ui_->telecom_path_line->text().isEmpty() || ui_->order_no_line->text().isEmpty() || ui_->order_quantity_line->text().isEmpty() ||
        ui_->data_line->text().isEmpty()) {
        QMessageBox::critical(this, "错误", "请先配置电信模板文件路径、订单号、订单数量、数据项单元格内容");
        return;
    }

    auto order_number  = ui_->telecom_order_combo_box->currentText().toStdString();
    auto generate_path = ui_->telecom_generate_path_line->text().toStdString();

    if (order_number.empty()) {
        QMessageBox::critical(this, "错误", "请输入订单号");
        return;
    }

    if (generate_path.empty()) {
        QMessageBox::critical(this, "错误", "请选择数据分配表路径");
        return;
    }

    loading_->setWindowTitle("电信数据分配表生成中...");
    loading_->show();

    auto template_path      = ui_->telecom_path_line->text().toStdString();
    auto generating_records = new GeneratingRecords(true, finance_db_, telecom_db_, ini_, order_number, "Iccid", template_path, generate_path);

    // 连接信号槽
    connect(generating_records, &GeneratingRecords::failure, this, &MainWindow::generatingRecordFailure);
    connect(generating_records, &GeneratingRecords::success, this, &MainWindow::generatingRecordSuccess);

    // 启动工作线程
    generating_records->start();
}

void MainWindow::selectFinanceTemplatePathBtnClicked() {
    QString file_path = QFileDialog::getOpenFileName(this, "选择金融数据分配表路径", "templates", "*.xlsx");

    if (file_path.isEmpty()) return;

    ui_->finance_path_line->setText(file_path);
}

void MainWindow::selectTelecomTemplatePathBtnClicked() {
    QString file_path = QFileDialog::getOpenFileName(this, "选择电信数据分配表路径", "templates", "*.xlsx");

    if (file_path.isEmpty()) return;

    ui_->telecom_path_line->setText(file_path);
}

void MainWindow::saveBtnClicked() {
    std::string mysql_host             = ui_->mysql_ip_line->text().toStdString();
    int         mysql_port             = ui_->mysql_port_line->text().toInt();
    std::string mysql_username         = ui_->mysql_username_line->text().toStdString();
    std::string mysql_password         = ui_->mysql_password_line->text().toStdString();
    std::string mysql_telecom_database = ui_->telecom_database_line->text().toStdString();
    std::string mysql_finance_database = ui_->finance_database_line->text().toStdString();
    std::string ftp_host               = ui_->ftp_ip_line->text().toStdString();
    int         ftp_port               = ui_->ftp_port_line->text().toInt();
    std::string ftp_username           = ui_->ftp_username_line->text().toStdString();
    std::string ftp_password           = ui_->ftp_password_line->text().toStdString();
    std::string remote_prd_path        = ui_->prd_line->text().toStdString();
    std::string remote_temp_path       = ui_->temp_line->text().toStdString();
    // std::string local_backup_path      = ui_->backup_line->text().toStdString();
    std::string finance_path   = ui_->finance_path_line->text().toStdString();
    std::string telecom_path   = ui_->telecom_path_line->text().toStdString();
    std::string order_no       = ui_->order_no_line->text().toStdString();
    std::string order_quantity = ui_->order_quantity_line->text().toStdString();
    std::string data           = ui_->data_line->text().toStdString();

    ini_.set("mysql", "host", mysql_host);
    ini_.set("mysql", "port", mysql_port);
    ini_.set("mysql", "username", mysql_username);
    ini_.set("mysql", "password", mysql_password);
    ini_.set("mysql", "telecom_database", mysql_telecom_database);
    ini_.set("mysql", "finance_database", mysql_finance_database);
    ini_.set("ftp", "host", ftp_host);
    ini_.set("ftp", "port", ftp_port);
    ini_.set("ftp", "username", ftp_username);
    ini_.set("ftp", "password", ftp_password);
    ini_.set("path", "remote_prd_path", remote_prd_path);
    ini_.set("path", "remote_temp_path", remote_temp_path);
    // ini_.set("path", "local_backup_path", local_backup_path);
    ini_.set("template", "finance_path", finance_path);
    ini_.set("template", "telecom_path", telecom_path);
    ini_.set("template", "order_no", order_no);
    ini_.set("template", "order_quantity", order_quantity);
    ini_.set("template", "data", data);

    if (ini_.save("config.ini")) {
        QMessageBox::information(this, "提示", "保存成功");
    } else {
        QMessageBox::critical(this, "错误", "保存失败");
    }
}

void MainWindow::confirmOrder(const std::string &confirm_datagram_dir_name) {
    order_window_->hide();

    loading_->setWindowTitle("订单处理中...");
    loading_->show();

    path_->order     = FilePath::join(path_->directory, confirm_datagram_dir_name);
    auto handleOrder = new HandleOrder(path_);

    // 连接信号槽
    connect(handleOrder, &HandleOrder::failure, this, &MainWindow::handleOrderFailure);
    connect(handleOrder, &HandleOrder::success, this, &MainWindow::handleOrderSuccess);

    // 启动工作线程
    handleOrder->start();
}

void MainWindow::cancelOrder() { order_window_->hide(); }

void MainWindow::confirmDeleteOrder(const std::string &password) {
    authenticator_->hide();

    if (password != "iflogic2025") {
        QMessageBox::critical(this, "错误", "密码错误");
        return;
    }

    // 弹出确认框
    auto order_no    = ui_->telecom_order_combo_box->currentText().toStdString();
    auto confirm_box = QMessageBox::question(this, "确认删除", "确认删除订单号为" + QString(order_no.c_str()) + "的订单吗?");

    if (confirm_box == QMessageBox::Yes) {
        Dms dms(telecom_db_, order_no);
        dms.deleteOrder();
        // 删除成功弹窗
        QMessageBox::information(this, "提示", "删除成功");
    }
}

void MainWindow::cancelDeleteOrder() { authenticator_->hide(); }

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
    loading_->hide();
    QMessageBox::critical(this, err_type, err_msg);
}

void MainWindow::uploadFileSuccess() {
    ui_->upload_temp_btn->setDisabled(false);
    loading_->hide();

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
    show_info();

    // // 显示路径
    // order_->showPath();

    // 打开复制按钮
    button_disabled(false);

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

void MainWindow::generatingRecordFailure() {
    loading_->hide();

    QMessageBox::critical(this, "错误", "请核对订单号与数据项是否正确");
}

void MainWindow::generatingRecordSuccess() {
    loading_->hide();

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

void MainWindow::resetCardFailure(const QString &err_msg) { QMessageBox::critical(this, "复位失败", err_msg); }

void MainWindow::resetCardSuccess(const QString &atr) {
    ui_->current_card_line->setText(atr);
    log_info(atr.toStdString().c_str());
}

void MainWindow::init_window() {
    // 设置窗口标题
    setWindowTitle("智能卡生产预处理软件");

    ui_->add_dir_widget->setAcceptDrops(false);
    setAcceptDrops(true);

    loading_ = new Loading(this);
}

void MainWindow::init_ui() {
    // // 使窗口始终在其他窗口之上
    // setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    button_disabled(true);
    std::string mysql_host             = ini_["mysql"]["host"];
    int         mysql_port             = ini_["mysql"]["port"];
    std::string mysql_username         = ini_["mysql"]["username"];
    std::string mysql_password         = ini_["mysql"]["password"];
    std::string mysql_finance_database = ini_["mysql"]["finance_database"];
    std::string mysql_telecom_database = ini_["mysql"]["telecom_database"];
    std::string ftp_host               = ini_["ftp"]["host"];
    int         ftp_port               = ini_["ftp"]["port"];
    std::string ftp_username           = ini_["ftp"]["username"];
    std::string ftp_password           = ini_["ftp"]["password"];
    std::string remote_prd_path        = ini_["path"]["remote_prd_path"];
    std::string remote_temp_path       = ini_["path"]["remote_temp_path"];
    std::string local_backup_path      = ini_["path"]["local_backup_path"];
    std::string finance_path           = ini_["template"]["finance_path"];
    std::string telecom_path           = ini_["template"]["telecom_path"];
    std::string order_no               = ini_["template"]["order_no"];
    std::string order_quantity         = ini_["template"]["order_quantity"];
    std::string data                   = ini_["template"]["data"];

    ui_->mysql_ip_line->setText(QString::fromStdString(mysql_host));
    ui_->mysql_port_line->setText(QString::number(mysql_port));
    ui_->mysql_username_line->setText(QString::fromStdString(mysql_username));
    ui_->mysql_password_line->setText(QString::fromStdString(mysql_password));
    ui_->finance_database_line->setText(QString::fromStdString(mysql_finance_database));
    ui_->telecom_database_line->setText(QString::fromStdString(mysql_telecom_database));
    ui_->ftp_ip_line->setText(QString::fromStdString(ftp_host));
    ui_->ftp_port_line->setText(QString::number(ftp_port));
    ui_->ftp_username_line->setText(QString::fromStdString(ftp_username));
    ui_->ftp_password_line->setText(QString::fromStdString(ftp_password));
    ui_->prd_line->setText(QString::fromStdString(remote_prd_path));
    ui_->temp_line->setText(QString::fromStdString(remote_temp_path));
    // ui_->backup_line->setText(QString::fromStdString(local_backup_path));
    ui_->finance_path_line->setText(QString::fromStdString(finance_path));
    ui_->telecom_path_line->setText(QString::fromStdString(telecom_path));
    ui_->order_no_line->setText(QString::fromStdString(order_no));
    ui_->order_quantity_line->setText(QString::fromStdString(order_quantity));
    ui_->data_line->setText(QString::fromStdString(data));

    // 读卡器类型
    ui_->reader_type_combo_box->addItem("PCSC");
    ui_->reader_type_combo_box->addItem("QSC");

    // 脚本运行器
    ui_->card_protocol_combo_box->addItem("ISO7816 (电信)");
    ui_->card_protocol_combo_box->addItem("GP (金融)");

    ui_->clear_card_btn->setDisabled(true);
    ui_->write_card_btn->setDisabled(true);
    ui_->start_auth_btn->setDisabled(true);
}

void MainWindow::init_signal_slot() {
    QClipboard *clip = QApplication::clipboard();

    connect(ui_->chinese_action, &QAction::triggered, this, &MainWindow::chineseLanguageAction);
    connect(ui_->english_action, &QAction::triggered, this, &MainWindow::englishLanguageAction);

    // 信息 - 订单信息
    connect(ui_->project_number_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->project_number.c_str())); });
    connect(ui_->order_number_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->order_number.c_str())); });
    connect(ui_->project_name_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->project_name.c_str())); });
    connect(ui_->chip_model_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->chip_model.c_str())); });
    connect(ui_->rf_code_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->rf_code.c_str())); });
    connect(ui_->script_package_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->script_package.c_str())); });

    // 信息 - 脚本包信息
    connect(ui_->open_personal_btn, &QPushButton::clicked, this, &MainWindow::openPersonalBtnClicked);
    connect(ui_->open_postpersonal_btn, &QPushButton::clicked, this, &MainWindow::openPostPersonalBtnClicked);
    connect(ui_->open_check_btn, &QPushButton::clicked, this, &MainWindow::openCheckBtnClicked);
    connect(ui_->open_clear_btn, &QPushButton::clicked, this, &MainWindow::openClearCardBtnClicked);

    // 鉴权
    connect(ui_->reader_type_combo_box, &QComboBox::currentTextChanged, this, &MainWindow::init_card_reader);
    connect(ui_->write_card_btn, &QPushButton::clicked, this, &MainWindow::writeCardBtnClicked);
    connect(ui_->clear_card_btn, &QPushButton::clicked, this, &MainWindow::clearCardBtnClicked);
    connect(ui_->reset_card_btn, &QPushButton::clicked, this, &MainWindow::resetCardBtnClicked);
    connect(ui_->open_auth_btn, &QPushButton::clicked, this, &MainWindow::openAuthScriptBtnClicked);
    connect(ui_->start_auth_btn, &QPushButton::clicked, this, &MainWindow::akaAuthBtnClicked);

    // 上传
    connect(ui_->upload_prd_btn, &QPushButton::clicked, this, &MainWindow::uploadPrdBtnClicked);
    connect(ui_->upload_temp_btn, &QPushButton::clicked, this, &MainWindow::uploadTempBtnClicked);

    // 制表
    connect(ui_->finance_select_generate_file_btn, &QPushButton::clicked, this, &MainWindow::selectFinanceGeneratePathBtnClicked);
    connect(ui_->finance_generating_btn, &QPushButton::clicked, this, &MainWindow::generatingFinanceRecordBtnClicked);
    connect(ui_->telecom_delete_order_btn, &QPushButton::clicked, this, &MainWindow::deleteTelecomOrderBtnClicked);
    connect(ui_->telecom_select_generate_file_btn, &QPushButton::clicked, this, &MainWindow::selectTelecomGeneratePathBtnClicked);
    connect(ui_->telecom_generating_btn, &QPushButton::clicked, this, &MainWindow::generatingTelecomRecordBtnClicked);

    // 配置
    connect(ui_->select_finance_template_file_btn, &QPushButton::clicked, this, &MainWindow::selectFinanceTemplatePathBtnClicked);
    connect(ui_->select_telecom_template_file_btn, &QPushButton::clicked, this, &MainWindow::selectTelecomTemplatePathBtnClicked);
    connect(ui_->save_btn, &QPushButton::clicked, this, &MainWindow::saveBtnClicked);
}

void MainWindow::init_config(const std::string &config_file) {
    if (!ini_.exists(config_file)) {
        ini_.set("log", "level", 0);

        ini_.set("mysql", "host", "127.0.0.1");
        ini_.set("mysql", "port", "3306");
        ini_.set("mysql", "username", "");
        ini_.set("mysql", "password", "");
        ini_.set("mysql", "telecom_database", "");
        ini_.set("mysql", "finance_database", "");

        ini_.set("ftp", "host", "127.0.0.1");
        ini_.set("ftp", "port", "21");
        ini_.set("ftp", "username", "");
        ini_.set("ftp", "password", "");

        ini_.set("path", "remote_prd_path", "");
        ini_.set("path", "remote_temp_path", "");
        ini_.set("path", "local_backup_path", "");

        ini_.set("template", "finance_path", "");
        ini_.set("template", "telecom_path", "");
        ini_.set("template", "order_no", "");
        ini_.set("template", "order_quantity", "");
        ini_.set("template", "data", "");

        ini_.set("qsc_card_reader", "count", 4);
        ini_.set("qsc_card_reader", "card_reader_1", "192.168.1.31:10002");
        ini_.set("qsc_card_reader", "card_reader_2", "192.168.1.31:10003");
        ini_.set("qsc_card_reader", "card_reader_3", "192.168.1.31:10004");
        ini_.set("qsc_card_reader", "card_reader_4", "192.168.1.31:10005");
        ini_.set("scsc_card_reader", "count", 4);
        ini_.set("scsc_card_reader", "card_reader_1", "192.168.1.31:10002");
        ini_.set("scsc_card_reader", "card_reader_2", "192.168.1.31:10003");
        ini_.set("scsc_card_reader", "card_reader_3", "192.168.1.31:10004");
        ini_.set("scsc_card_reader", "card_reader_4", "192.168.1.31:10005");

        if (!ini_.save(config_file)) {
            QMessageBox::critical(this, "错误", "配置文件保存失败");
        }
    } else {
        if (!ini_.load(config_file)) {
            QMessageBox::critical(this, "错误", "配置文件加载失败");
        }
    }
}

void MainWindow::init_logger(const std::string &log_file) {
    if (!Logger::instance().isOpen()) {
        Logger::instance().open(log_file);
        Logger::instance().setFormat(false);
        int level = ini_["log"]["level"];
        Logger::instance().setLevel(Logger::Level(level));
    }
}

void MainWindow::init_card_reader() {
    data_handler_ = DH_Create();

    int                      reader_type = ui_->reader_type_combo_box->currentIndex();
    std::vector<std::string> connect_infos;
    switch (reader_type) {

    case 1: {
        int count = ini_["qsc_card_reader"]["count"];
        for (int i = 1; i <= count; i++) {
            std::string ip_port = ini_["qsc_card_reader"]["card_reader_" + std::to_string(i)];
            connect_infos.push_back(ip_port);
        }
        break;
    }

    case 2: {
        int count = ini_["scsc_card_reader"]["count"];
        for (int i = 1; i <= count; i++) {
            std::string ip_port = ini_["scsc_card_reader"]["card_reader_" + std::to_string(i)];
            connect_infos.push_back(ip_port);
        }
        break;
    }

    default:
        break;
    }

    int protocol = ui_->card_protocol_combo_box->currentIndex();

    try {
        const char *readers[16]  = {};
        int         reader_count = connect_infos.size();
        for (int i = 0; i < reader_count; i++) {
            readers[i] = connect_infos[i].c_str();
        }

        bool ret = DH_Initialize(data_handler_, reader_type, readers, &reader_count);
        if (!ret) {
            throw std::exception();
        }

        ret = DH_CardProtocol(data_handler_, protocol);
        if (!ret) {
            throw std::exception();
        }

        ui_->reader_combo_box->clear();
        for (int i = 0; i < reader_count; i++) {
            ui_->reader_combo_box->addItem(readers[i]);
        }

        ui_->reset_card_btn->setDisabled(false);
    } catch (std::exception &e) {
        QMessageBox::critical(this, "警告", "读卡器初始化，请检查读卡器是否连接");
        ui_->reset_card_btn->setDisabled(true);
        return;
    }
}

void MainWindow::init_database() {
    finance_db_ = std::make_shared<zel::myorm::Database>();
    if (finance_db_->connect(ini_["mysql"]["host"], ini_["mysql"]["port"], ini_["mysql"]["username"], ini_["mysql"]["password"],
                             ini_["mysql"]["finance_database"])) {
        Tabulation tabulation(finance_db_, telecom_db_, ini_);
        auto       finance_order_list = tabulation.financeOrderList();
        reverse(finance_order_list.begin(), finance_order_list.end());
        QStringList finance_items;
        for (auto order : finance_order_list) {
            finance_items.append(QString::fromStdString(order));
        }

        ui_->finance_order_combo_box->addItems(finance_items);
        ui_->finance_order_combo_box->setCurrentText("");
    } else {
        log_error("Failed to connect to database");
        ui_->finance_generating_btn->setDisabled(true);
    }

    telecom_db_ = std::make_shared<zel::myorm::Database>();
    if (telecom_db_->connect(ini_["mysql"]["host"], ini_["mysql"]["port"], ini_["mysql"]["username"], ini_["mysql"]["password"],
                             ini_["mysql"]["telecom_database"])) {
        Tabulation tabulation(finance_db_, telecom_db_, ini_);
        auto       telecom_order_list = tabulation.telecomOrderList();
        reverse(telecom_order_list.begin(), telecom_order_list.end());
        QStringList telecom_items;
        for (auto order : telecom_order_list) {
            telecom_items.append(QString::fromStdString(order));
        }

        ui_->telecom_order_combo_box->addItems(telecom_items);
        ui_->telecom_order_combo_box->setCurrentText("");
    } else {
        log_error("Failed to connect to database");
        ui_->telecom_generating_btn->setDisabled(true);
    }
}

void MainWindow::init_auth_script(const std::string &auth_script_path) {
    std::string auth_script = R"(
// run_gp_apdu 对APDU返回值进行特殊处理
run_gp_apdu = func (apdu) {
    resp = apdu -> null // 运行APDU命令, 返回 hash = {"data": "xxxx", "sw1": "90", "sw2": "00", "sw": "9000"}

    if resp.sw1 == "61" {
        gr = "00C00000" + resp.sw2 -> "*9000" 
        return gr       
    }

    if resp.sw == "9000" {
        return resp
    }

   return panic("Unknown response:" + resp)
}

// xor 异或运算 a: 字节数组, b: 字节数组
xor = func (a, b) {
    for i = 0; i < a.len(); i++ {
        a[i] ^= b[i]
    }
    return a
}

// hex_to_bytes 将十六进制字符串转换为字节数组
hex_to_bytes = func (hex) {
    bytes = []

    // 初始化字节数组
    for i = 0; i < hex.len(); i += 2 {
        bytes.append(0)
    }

    for i = 0; i < hex.len(); i += 2 {
        index = i / 2
        bytes[index] = int(hex.mid(i, 2), 16)
    }
    return bytes
}

// bytes_to_hex 将字节数组转换为十六进制字符串
bytes_to_hex = func (bytes) {
    hex = ""
    for b in bytes {
        hex += b.toHexString()
    }
    return hex
}

// tlv_find_tag 在TLV数据中查找指定标签的TLV项
tlv_find_tag = func (tlvs, tag) {
    for t in tlvs {
        if t.tag == tag {
            return t 
        }
    }
    return null
}

// authentication 鉴权过程
authentication = func (SQN) {
    atr = RST -> null

    // 1. 选择 MF
    run_gp_apdu("00A40004023F00") 

    // 2. 选择 EF.DIR
    resp = run_gp_apdu("00A40004022F00")

    // 2.1 读取 EF.DIR
    tlvs = tlv.parse(resp.data)
    file_descriptor = tlv_find_tag(tlvs, "82").value
    record_length = file_descriptor.mid(6,2)
    resp = "00B20104" + record_length -> "*9000"

    
    // 3. 获取 USIM AID
    /* tlv.parse(tlv_data) 解析TLV数据, 返回列表 
        tlvs = [
            {"value": "4F10A0000000871002FF49FFFF89040B00FF50045553494D", "length": 24, "tag": "61"}, 
            {"value": "A0000000871002FF49FFFF89040B00FF", "length": 16, "tag": "4F"}, 
            {"value": "5553494D", "length": 4, "tag": "50"}
        ] 
    */
    tlvs = tlv.parse(resp.data)
    AID = tlv_find_tag(tlvs, "4F")
    run_gp_apdu("00A40404" + AID.length.toHexString() + AID.value)

    // 5. AKA 鉴权
    RAND = crypto.randomHex(32)
    AMF = "0000"

    /* crypto.milenage(KI, OPC, RAND, SQN, AMF) 计算 Milenage 值, 返回 hash
    output = {
        "MacA": "6F619D641724807F", 
        "AK": "3341F0BAD810", 
        "MacS": "6040C2CD484C027C", 
        "IK": "7A41C1B0719D3B6F81FEB6DF74877B84", 
        "RES": "82AF78C2D3E4C090", 
        "CK": "86A565BEFDE46FB2A4F38A0DAE51585C", 
        "AKStar": "7E9AC6C597FA"
    }*/
    output = crypto.milenage(ds.KI, ds.OPC, RAND, SQN, AMF)

    // 6. 构建鉴权命令
    sqn_bytes = hex_to_bytes(SQN)
    amf_bytes = hex_to_bytes(AMF)
    ak_bytes = hex_to_bytes(output.AK)
    mac_a_bytes = hex_to_bytes(output.MacA)

    auth_bytes = xor(sqn_bytes, ak_bytes)
    auth_bytes.append(amf_bytes[0])
    auth_bytes.append(amf_bytes[1])
    auth_bytes.extend(mac_a_bytes)
    AUTH = bytes_to_hex(auth_bytes)

    // 7. 发送鉴权命令
    resp = run_gp_apdu("0088008122" + (RAND.len() / 2).toHexString() + RAND + (AUTH.len() / 2).toHexString() + AUTH + "00").data
    return {
        "resp": resp,
        "AKStar": output.AKStar
    }
}

// caculate_sqn 计算 SQN
caculate_sqn = func (AUTS, AKStar) {
    auts_bytes = hex_to_bytes(AUTS.mid(0, 12))
    akstar_bytes = hex_to_bytes(AKStar)

    // SQN_MS = SQNxorAKs XOR AKStar
    /* sqn_ms_bytes = []
     for i = 0; i < 6; i++ {
        sqn_ms_bytes.append(auts_bytes[i] ^ akstar_bytes[i])
    }*/
    sqn_ms_bytes = xor(auts_bytes, akstar_bytes)

    // SQN = SQN_MS + 1
    sqn_bytes = sqn_ms_bytes
    for i = 5; i >= 0; i-- {
        if sqn_bytes[i] == 255 {
            sqn_bytes[i] = 0
        } else {
            sqn_bytes[i]++
            break
        }
    }

    return bytes_to_hex(sqn_ms_bytes)
}

// 验证 PIN
verify_pin = func () {
     resp = "0020000108" + ds.PIN1 + "00" -> null
     print(resp)
     return (resp.sw == "9000" || resp.sw == "6984")
}

// 激活 PIN 
activate_pin = func () {
    resp = "0028000108" + ds.PIN1 -> null
    print(resp)
    return (resp.sw == "9000")
}

// ------------------------------------------ 脚本开始 ------------------------------
atr = RST -> null
if atr.mid(atr.len() -4, 4) == "9000" {
    panic("The cards are not personalized")
}

if !verify_pin() {
    panic("PIN verification failed")
} 

// 首次鉴权
SQN = "000000000020"
result = authentication(SQN)
switch result.resp.mid(0,2) {
    case "DB": {
        print("Authentication successful")
        break
    }

    case "DC": {
        // 计算 SQN
        SQN = caculate_sqn(result.resp.mid(4, result.resp.len() - 4), result.AKStar)

        // 重试鉴权
        if authentication(SQN).resp.mid(0,2) != "DB" {
            panic("Authentication failed")
        } else {
            print("Authentication successful")
        }
        break
    }

    default: {
        panic("Unknown error")
        break
    }
}
    )";

    // 写入文件
    std::ofstream ofs(auth_script_path, std::ios::out);
    ofs << auth_script;
    ofs.close();
}

void MainWindow::button_disabled(bool disabled) {
    ui_->project_number_btn->setDisabled(disabled);
    ui_->order_number_btn->setDisabled(disabled);
    ui_->project_name_btn->setDisabled(disabled);
    ui_->chip_model_btn->setDisabled(disabled);
    ui_->rf_code_btn->setDisabled(disabled);
    ui_->script_package_btn->setDisabled(disabled);
    ui_->open_auth_btn->setDisabled(disabled);
    ui_->upload_prd_btn->setDisabled(disabled);
    ui_->open_personal_btn->setDisabled(disabled);
    ui_->open_postpersonal_btn->setDisabled(disabled);
    ui_->open_check_btn->setDisabled(disabled);
    ui_->open_clear_btn->setDisabled(disabled);
    ui_->write_card_btn->setDisabled(disabled);
    ui_->clear_card_btn->setDisabled(disabled);
    ui_->upload_temp_btn->setDisabled(disabled);
}

void MainWindow::show_info() {
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

    ui_->auth_script_line->setText(QString("./auth.script"));

    ui_->c1_line->setText("0");
    ui_->c2_line->setText("1");
    ui_->c3_line->setText("2");
    ui_->c4_line->setText("4");
    ui_->c5_line->setText("8");

    ui_->r1_line->setText("64");
    ui_->r2_line->setText("0");
    ui_->r3_line->setText("32");
    ui_->r4_line->setText("64");
    ui_->r5_line->setText("96");

    ui_->ds_check_box->setChecked(script_info_->has_ds);
    ui_->start_auth_btn->setDisabled(false);
}

void MainWindow::switch_language(const QString &language_file) {
    qApp->removeTranslator(&translator_);

    if (translator_.load(":/translation/" + language_file + ".qm")) {
        qApp->installTranslator(&translator_);
        current_lang_ = language_file;
        retranslate_ui();
    } else {
        qDebug() << "加载语言失败：" << language_file;
    }
}

void MainWindow::retranslate_ui() { ui_->retranslateUi(this); }