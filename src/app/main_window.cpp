#include "main_window.h"

#include "tabulation/tabulation.h"
#include "task/handle_order.hpp"
#include "task/run_script.hpp"
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
#include <qicon.h>
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

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_MainWindow)
    , order_info_(nullptr)
    , person_data_info_(nullptr)
    , script_info_(nullptr)
    , card_device_(nullptr) {
    ui_->setupUi(this);

    // 初始化窗口
    init_window();

    // 初始化配置
    init_config("config.ini");

    // 初始化产业部脚本信息
    init_script_info("scripts.json");

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
    init_auth_script("scripts/auth.if");
}

MainWindow::~MainWindow() {
    APP_Destroy(card_device_);
    delete ui_;
}

void MainWindow::chineseLanguageAction() { switch_language("zh_CN"); }

void MainWindow::englishLanguageAction() { switch_language("en_US"); }

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.empty()) return;

    std::string datagram_path = urls.first().toLocalFile().toStdString();

    loading_->setWindowTitle("订单处理中...");
    loading_->show();

    auto handleOrder = new HandleOrder(datagram_path);

    // 连接信号槽
    connect(handleOrder, &HandleOrder::failure, this, &MainWindow::handleOrderFailure);
    connect(handleOrder, &HandleOrder::success, this, &MainWindow::handleOrderSuccess);

    // 启动工作线程
    handleOrder->start();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void MainWindow::openPersonalBtnClicked() { QDesktopServices::openUrl(QUrl::fromLocalFile(QString(script_info_->person_path.c_str()))); }

void MainWindow::openPostPersonalBtnClicked() { QDesktopServices::openUrl(QUrl::fromLocalFile(QString(script_info_->post_person_path.c_str()))); }

void MainWindow::openCheckBtnClicked() { QDesktopServices::openUrl(QUrl::fromLocalFile(QString(script_info_->check_path.c_str()))); }

void MainWindow::openClearCardBtnClicked() { QDesktopServices::openUrl(QUrl::fromLocalFile(QString(script_info_->clear_path.c_str()))); }

void MainWindow::runScriptBtnClicked() {
    ui_->result_line_edit->setText(tr("正在读取 ..."));

    auto        currentName = ui_->script_type_combo_box->currentText().toStdString();
    std::string currentPath;
    for (size_t i = 0; i < script_->size(); ++i) {
        const zel::json::Json &item = (*script_)[i];
        std::string            name = item["name"].asString();
        std::string            path = item["path"].asString();
        if (currentName == name) {
            currentPath = path;
        }
    }

    auto run_script = new RunScript(currentName, currentPath, ui_->reader_combo_box->currentIndex(), card_device_);

    connect(run_script, &RunScript::success, this, &MainWindow::runScriptSuccess);
    connect(run_script, &RunScript::failure, this, &MainWindow::runScriptFailure);

    run_script->start();
}

void MainWindow::writeCardBtnClicked() {
    // 弹出写卡加载窗口, 并停留
    WriteCardLoading *write_card_loading = new WriteCardLoading(this);
    write_card_loading->show();

    // 创建工作线程
    auto write_card = new WriteCard(script_info_, person_data_info_, ui_->reader_combo_box->currentIndex(), card_device_);

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
    auto clear_card = new ClearCard(script_info_, person_data_info_, ui_->reader_combo_box->currentIndex(), card_device_, script_convert);
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
    auto aka_auth = new AkaAuth(script_info_, person_data_info_, ui_->reader_combo_box->currentIndex(), card_device_);
    // 连接信号槽
    connect(aka_auth, &AkaAuth::failure, aka_auth_loading, &AkaAuthLoading::failure);
    connect(aka_auth, &AkaAuth::success, aka_auth_loading, &AkaAuthLoading::success);

    // 启动工作线程
    aka_auth->start();
}

void MainWindow::uploadPrdBtnClicked() {
    // loading_->setWindowTitle("正在上传个人化数据...");
    // loading_->show();

    // // 将个人化数据上传到FTP服务器
    // std::string remote_prd_path = ini_["path"]["remote_prd_path"].asString() + "/" + order_info_->project_name;
    // std::string local_prd_path  = path_->data;
    // auto        upload_file     = new UploadFile(ini_, local_prd_path, remote_prd_path, false, path_);

    // // 连接信号槽
    // connect(upload_file, &UploadFile::failure, this, &MainWindow::uploadFileFailure);
    // connect(upload_file, &UploadFile::success, this, &MainWindow::uploadFileSuccess);

    // // 启动工作线程
    // upload_file->start();
}

void MainWindow::uploadTempBtnClicked() {
    // loading_->setWindowTitle("正在上传临时文件...");
    // loading_->show();

    // std::string remote_temp_path = ini_["path"]["remote_temp_path"];
    // std::string local_temp_path  = FilePath::dir(path_->temp);
    // auto        upload_prd       = new UploadFile(ini_, local_temp_path, remote_temp_path, true, path_);

    // // 连接信号槽
    // connect(upload_prd, &UploadFile::failure, this, &MainWindow::uploadFileFailure);
    // connect(upload_prd, &UploadFile::success, this, &MainWindow::uploadFileSuccess);

    // // 启动工作线程
    // upload_prd->start();
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

void MainWindow::runScriptFailure(const QString &err_msg) {
    QMessageBox::critical(this, "读取失败", err_msg);
    ui_->result_line_edit->clear();
}

void MainWindow::runScriptSuccess(const QString &script_name, const QString &result) {
    run_result_ = result.toStdString();

    if (script_name.indexOf("ICCID") != -1 && result.indexOf("失败") == -1) {
        ui_->result_line_edit->setText(swap(result));
    } else if (script_name.indexOf("PAN") != -1 && result.indexOf("失败") == -1) {
        QString display_pan;
        for (int i = 0; i < result.length(); i++) {
            if (i % 4 == 0 && i != 0) {
                display_pan += "-";
            }

            if (i > 5 && i < 12) {
                display_pan += "X";
            } else {
                display_pan += result[i];
            }
        }

        ui_->result_line_edit->setText(display_pan);
    } else {
        ui_->result_line_edit->setText(result);
    }
}

void MainWindow::init_window() {
    // 设置窗口标题
    setWindowTitle("智能卡生产预处理软件 v3.5.1");

    ui_->add_dir_widget->setAcceptDrops(false);
    setAcceptDrops(true);

    loading_ = new Loading(this);
}

void MainWindow::init_ui() {
    // // 使窗口始终在其他窗口之上
    // setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    button_disabled(true);
    const QSize button_icon_size(16, 16);
    auto        set_button_icon = [&](QPushButton *button, const char *icon_path) {
        button->setIcon(QIcon(icon_path));
        button->setIconSize(button_icon_size);
    };

    set_button_icon(ui_->order_number_btn, ":/image/copy.svg");
    set_button_icon(ui_->project_name_btn, ":/image/copy.svg");
    set_button_icon(ui_->product_type_btn, ":/image/copy.svg");
    set_button_icon(ui_->rf_code_btn, ":/image/copy.svg");
    set_button_icon(ui_->script_package_btn, ":/image/copy.svg");
    set_button_icon(ui_->chip_model_btn, ":/image/copy.svg");
    set_button_icon(ui_->open_personal_btn, ":/image/folder-open.svg");
    set_button_icon(ui_->open_postpersonal_btn, ":/image/folder-open.svg");
    set_button_icon(ui_->open_check_btn, ":/image/folder-open.svg");
    set_button_icon(ui_->open_clear_btn, ":/image/folder-open.svg");
    set_button_icon(ui_->write_card_btn, ":/image/credit-card.svg");
    set_button_icon(ui_->clear_card_btn, ":/image/eraser.svg");
    set_button_icon(ui_->start_auth_btn, ":/image/play.svg");
    set_button_icon(ui_->finance_select_generate_file_btn, ":/image/folder-open.svg");
    set_button_icon(ui_->finance_generating_btn, ":/image/file-spreadsheet.svg");
    set_button_icon(ui_->telecom_delete_order_btn, ":/image/trash-2.svg");
    set_button_icon(ui_->telecom_select_generate_file_btn, ":/image/folder-open.svg");
    set_button_icon(ui_->telecom_generating_btn, ":/image/file-spreadsheet.svg");
    set_button_icon(ui_->select_finance_template_file_btn, ":/image/folder-open.svg");
    set_button_icon(ui_->select_telecom_template_file_btn, ":/image/folder-open.svg");
    set_button_icon(ui_->save_btn, ":/image/save.svg");

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
    // ui_->backup_line->setText(QString::fromStdString(local_backup_path));
    ui_->finance_path_line->setText(QString::fromStdString(finance_path));
    ui_->telecom_path_line->setText(QString::fromStdString(telecom_path));
    ui_->order_no_line->setText(QString::fromStdString(order_no));
    ui_->order_quantity_line->setText(QString::fromStdString(order_quantity));
    ui_->data_line->setText(QString::fromStdString(data));

    // 读卡器类型
    ui_->reader_type_combo_box->addItem("PC/SC");
    ui_->reader_type_combo_box->addItem("Q/SC");
    ui_->reader_type_combo_box->addItem("SC/SC");
    ui_->reader_type_combo_box->addItem("PT/SC");

    // 脚本运行器
    ui_->card_protocol_combo_box->addItem("ISO7816 (电信)");
    ui_->card_protocol_combo_box->addItem("GP (金融)");

    set_button_icon(ui_->run_btn, ":/image/play.svg");

    for (size_t i = 0; i < script_->size(); ++i) {
        const zel::json::Json &item = (*script_)[i];
        std::string            name = item["name"].asString();
        ui_->script_type_combo_box->addItem(name.c_str());
    }
}

void MainWindow::init_signal_slot() {
    QClipboard *clip = QApplication::clipboard();

    connect(ui_->chinese_action, &QAction::triggered, this, &MainWindow::chineseLanguageAction);
    connect(ui_->english_action, &QAction::triggered, this, &MainWindow::englishLanguageAction);

    // 信息 - 订单信息
    connect(ui_->order_number_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->order_number.c_str())); });
    connect(ui_->project_name_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->project_name.c_str())); });
    connect(ui_->product_type_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->product_type.c_str())); });
    connect(ui_->rf_code_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->rf_code.c_str())); });
    connect(ui_->script_package_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->script_package.c_str())); });
    connect(ui_->chip_model_btn, &QPushButton::clicked, [=]() { clip->setText(QString(order_info_->chip_model.c_str())); });

    // 信息 - 脚本包信息
    connect(ui_->open_personal_btn, &QPushButton::clicked, this, &MainWindow::openPersonalBtnClicked);
    connect(ui_->open_postpersonal_btn, &QPushButton::clicked, this, &MainWindow::openPostPersonalBtnClicked);
    connect(ui_->open_check_btn, &QPushButton::clicked, this, &MainWindow::openCheckBtnClicked);
    connect(ui_->open_clear_btn, &QPushButton::clicked, this, &MainWindow::openClearCardBtnClicked);

    // 鉴权
    connect(ui_->reader_type_combo_box, &QComboBox::currentTextChanged, this, &MainWindow::init_card_reader);
    connect(ui_->write_card_btn, &QPushButton::clicked, this, &MainWindow::writeCardBtnClicked);
    connect(ui_->clear_card_btn, &QPushButton::clicked, this, &MainWindow::clearCardBtnClicked);
    connect(ui_->run_btn, &QPushButton::clicked, this, &MainWindow::runScriptBtnClicked);
    // connect(ui_->open_auth_btn, &QPushButton::clicked, this, &MainWindow::openAuthScriptBtnClicked);
    connect(ui_->start_auth_btn, &QPushButton::clicked, this, &MainWindow::akaAuthBtnClicked);

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
        ini_.set("log", "level", 3);

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
    card_device_ = APP_Create();

    int                      reader_type = ui_->reader_type_combo_box->currentIndex();
    std::vector<std::string> connect_infos;
    int                      protocol     = ui_->card_protocol_combo_box->currentIndex();
    const char              *readers[255] = {};
    int                      reader_count = 255;

    try {
        bool ret = APP_Initialize(card_device_, reader_type, readers, &reader_count);
        if (!ret) {
            char error[1024];
            APP_GetLastError(card_device_, error, sizeof(error));
            throw std::exception(error);
        }

        ret = APP_CardProtocol(card_device_, protocol);
        if (!ret) {
            char error[1024];
            APP_GetLastError(card_device_, error, sizeof(error));
            throw std::exception(error);
        }

        ui_->reader_combo_box->clear();
        for (int i = 0; i < reader_count; i++) {
            ui_->reader_combo_box->addItem(readers[i]);
        }

        ui_->run_btn->setDisabled(false);
    } catch (std::exception &e) {
        QMessageBox::critical(this, "警告", "读卡器初始化，请检查读卡器是否连接, 错误信息:" + QString::fromStdString(e.what()));
        ui_->run_btn->setDisabled(true);
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
    std::string auth_script = R"(// run_gp_apdu 对APDU返回值进行特殊处理
run_gp_apdu = func(apdu) {
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
xor = func(a, b) {
    for i = 0; i < a.len(); i++ {
        a[i] ^= b[i]
    }
    return a
}

// hex_to_bytes 将十六进制字符串转换为字节数组
hex_to_bytes = func(hex) {
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
bytes_to_hex = func(bytes) {
    hex = ""
    for b in bytes {
        hex += b.toHexString()
    }
    return hex
}

// authentication 鉴权过程
authentication = func(SQN) {
    atr = RST -> null

    // 1. 选择 MF
    run_gp_apdu("00A40004023F00")

    // 2. 选择 EF.DIR
    resp = run_gp_apdu("00A40004022F00")

    // 2.1 读取 EF.DIR
    tlvs = Tlv.parse(resp.data)
    file_descriptor = Tlv.find(tlvs, "82").value
    record_length = file_descriptor.mid(6, 2)
    resp = "00B20104" + record_length -> "*9000"

    // 3. 获取 USIM AID
    /* Tlv.parse(tlv_data) 解析TLV数据, 返回列表
    tlvs = [
    {"value": "4F10A0000000871002FF49FFFF89040B00FF50045553494D", "length": 24, "tag": "61"},
    {"value": "A0000000871002FF49FFFF89040B00FF", "length": 16, "tag": "4F"},
    {"value": "5553494D", "length": 4, "tag": "50"}
    ]
    */
    tlvs = Tlv.parse(resp.data)
    AID = Tlv.find(tlvs, "4F")
    run_gp_apdu("00A40404" + AID.length.toHexString() + AID.value)

    // 5. AKA 鉴权
    RAND = Crypto.randomHex(32)
    AMF = "0000"

    /* Crypto.milenage(KI, OPC, RAND, SQN, AMF) 计算 Milenage 值, 返回 hash
    output = {
    "MacA": "6F619D641724807F",
    "AK": "3341F0BAD810",
    "MacS": "6040C2CD484C027C",
    "IK": "7A41C1B0719D3B6F81FEB6DF74877B84",
    "RES": "82AF78C2D3E4C090",
    "CK": "86A565BEFDE46FB2A4F38A0DAE51585C",
    "AKStar": "7E9AC6C597FA"
    }*/
    output = Crypto.milenage(ds.KI, ds.OPC, RAND, SQN, AMF)

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
    resp = run_gp_apdu("0088008122" + (RAND.len() / 2).toHexString() + RAND + (AUTH.len() / 2).toHexString() + AUTH).data
    return {
        "resp": resp,
        "AKStar": output.AKStar
    }
}

// caculate_sqn 计算 SQN
caculate_sqn = func(AUTS, AKStar) {
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
verify_pin = func() {
    resp = "0020000108" + ds.PIN1 -> null
    print(resp)
    return(resp.sw == "9000" || resp.sw == "6984")
}

// 激活 PIN
activate_pin = func() {
    resp = "0028000108" + ds.PIN1 -> null
    print(resp)
    return(resp.sw == "9000")
}

// ------------------------------------------ 脚本开始 ------------------------------
atr = RST -> null
if atr.mid(atr.len() - 4, 4) == "9000" {
    panic("The cards are not personalized")
}

if ! verify_pin() {
    panic("PIN verification failed")
}

// 首次鉴权
SQN = "000000000020"
result = authentication(SQN)
switch result.resp.mid(0, 2) {
case "DB": {
    print("Authentication successful")
    break
}

case "DC": {
    // 计算 SQN
    SQN = caculate_sqn(result.resp.mid(4, result.resp.len() - 4), result.AKStar)

    // 重试鉴权
    if authentication(SQN).resp.mid(0, 2) != "DB" {
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
})";

    // 写入文件
    zel::fs::File script(auth_script_path);
    script.create();
    script.write(auth_script);
}

void MainWindow::init_script_info(const std::string &script_json) {
    zel::json::Json json;
    zel::fs::File   script(script_json);

    if (!script.exists()) {
        auto            path = zel::fs::join(script.dirPath(), zel::fs::join("scripts", "atr.if"));
        zel::json::Json atr;
        atr["name"] = "ATR-[通用]";
        atr["path"] = "scripts/atr.if";
        zel::fs::File atr_if(path);
        std::string   content = R"(atr = RST -> null
print(atr))";
        atr_if.create();
        atr_if.write(content);

        path = zel::fs::join(script.dirPath(), zel::fs::join("scripts", "pan.if"));
        zel::json::Json pan;
        pan["name"] = "PAN-[金融]";
        pan["path"] = "scripts/pan.if";
        zel::fs::File pan_if(path);
        content = R"(RST -> null
resp = "00A404000E325041592E5359532E4444463031" -> "*9000"
tlvs = tlv.parse(resp.data)
aid = tlv.find(tlvs, "4F")
if type(aid) == "null" {
	panic("获取AID失败")
}
"00A40400" + aid.length.toHexString() + aid.value -> "*9000"
resp = "00B2051400" -> "*9000"
tlvs = tlv.parse(resp.data)
pan = tlv.find(tlvs, "5A")
if type(pan) == "null" {
	print("未查询到卡号")
} else {
	print(pan.value)
})";
        pan_if.create();
        pan_if.write(content);

        path = zel::fs::join(script.dirPath(), zel::fs::join("scripts", "iccid.if"));
        zel::json::Json iccid;
        iccid["name"] = "ICCID-[电信]";
        iccid["path"] = "scripts/iccid.if";
        zel::fs::File iccid_if(path);
        content = R"(RST -> null
"A0A40000023F00" -> null
"A0A40000022FE2" -> null
iccid = "A0B000000A" -> null
if iccid.data == "" {
	print("获取卡片 ICCID 失败, 返回值:", iccid.sw)
} else {
	print(iccid.data)
})";
        iccid_if.create();
        iccid_if.write(content);

        path = zel::fs::join(script.dirPath(), zel::fs::join("scripts", "imsi.if"));
        zel::json::Json imsi;
        imsi["name"] = "IMSI-[电信]";
        imsi["path"] = "scripts/imsi.if";
        zel::fs::File imsi_if(path);
        content = R"(RST -> null
"A0A40000027F20" -> null
"A0A40000026F07" -> null
imsi = "A0B0000009" -> null
if imsi.data == "" {
	print("获取卡片 IMSI 失败, 返回值:", imsi.sw)
} else {
	print(imsi.data)
})";
        imsi_if.create();
        imsi_if.write(content);

        path = zel::fs::join(script.dirPath(), zel::fs::join("scripts", "puk.if"));
        zel::json::Json puk;
        puk["name"] = "PUK-[电信]";
        puk["path"] = "scripts/puk.if";
        zel::fs::File puk_if(path);
        content = R"(RST -> null
"A0A40000022F00" -> null
"A0A40000027F20" -> null
"A0A40000026F78" -> null
puk = "A0B0000002" -> null
if puk.data == "" {
	print("获取卡片 PUK 失败, 返回值:", puk.sw)
} else {
	print(puk.data)
})";
        puk_if.create();
        puk_if.write(content);

        auto script_array = zel::json::Json::array();
        script_array.append(atr);
        script_array.append(pan);
        script_array.append(iccid);
        script_array.append(imsi);
        script_array.append(puk);

        script.create();
        script.write(script_array.dump());
    }

    json.load(script_json);
    script_ = std::make_shared<zel::json::Json>(json);
}

void MainWindow::button_disabled(bool disabled) {
    ui_->order_number_btn->setDisabled(disabled);
    ui_->project_name_btn->setDisabled(disabled);
    ui_->product_type_btn->setDisabled(disabled);
    ui_->rf_code_btn->setDisabled(disabled);
    ui_->script_package_btn->setDisabled(disabled);
    ui_->chip_model_btn->setDisabled(disabled);
    ui_->open_personal_btn->setDisabled(disabled);
    ui_->open_postpersonal_btn->setDisabled(disabled);
    ui_->open_check_btn->setDisabled(disabled);
    ui_->open_clear_btn->setDisabled(disabled);

    if (person_data_info_ != nullptr && script_info_ != nullptr) {
        ui_->start_auth_btn->setDisabled(false);
        ui_->write_card_btn->setDisabled(false);
        ui_->clear_card_btn->setDisabled(false);
    } else {
        ui_->start_auth_btn->setDisabled(true);
        ui_->write_card_btn->setDisabled(true);
        ui_->clear_card_btn->setDisabled(true);
    }
}

void MainWindow::show_info() {
    if (order_info_ != nullptr) {
        ui_->order_number_line->setText(QString(order_info_->order_number.c_str()));
        ui_->order_number_line->setCursorPosition(0);
        ui_->project_name_line->setText(QString(order_info_->project_name.c_str()));
        ui_->product_type_line->setText(QString(order_info_->product_type.c_str()));
        ui_->rf_code_line->setText(QString(order_info_->rf_code.c_str()));
        ui_->script_package_line->setText(QString(order_info_->script_package.c_str()));
        ui_->script_package_line->setCursorPosition(0);
        ui_->chip_model_line->setText(QString(order_info_->chip_model.c_str()));
    } else {
        ui_->order_number_line->clear();
        ui_->project_name_line->clear();
        ui_->product_type_line->clear();
        ui_->rf_code_line->clear();
        ui_->script_package_line->clear();
        ui_->chip_model_line->clear();
    }

    ui_->person_script_line->setText(QString(script_info_->person_filename.c_str()));
    ui_->person_script_line->setCursorPosition(0);
    ui_->post_person_script_line->setText(QString(script_info_->post_person_filename.c_str()));
    ui_->post_person_script_line->setCursorPosition(0);
    ui_->check_script_line->setText(QString(script_info_->check_filename.c_str()));
    ui_->check_script_line->setCursorPosition(0);
    ui_->clear_script_line->setText(QString(script_info_->clear_filename.c_str()));
    ui_->clear_script_line->setCursorPosition(0);

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

QString MainWindow::swap(QString str) {
    // 长度必须为偶数
    if (str.size() % 2 != 0) {
        return QString(); // 返回空串表示非法
    }

    QString result = str;

    for (int i = 0; i < result.size(); i += 2) {
        QChar tmp     = result[i];
        result[i]     = result[i + 1];
        result[i + 1] = tmp;
    }

    return result;
}
