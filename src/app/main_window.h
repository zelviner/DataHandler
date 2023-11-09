#pragma once
#include "public/ftp/ftp.h"
#include "public/utility/ini_file.h"
#include "info/order.h"
#include "info/person_data.h"
#include "info/script.h"
#include "ui_main_window.h"

#include <QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QMainWindow *parent = nullptr);
    ~MainWindow();

    /// @brief 获取订单全部信息
    bool getInfo(QString &error);

    /// @brief 处理订单
    bool doOrder(QString &error);

  private:
    /// @brief 初始化窗口
    void initWindow();

    /// @brief 初始化UI
    void initUI();

    /// @brief 初始化信号槽
    void initSignalSlot();

    /// @brief 初始化配置
    void initConfig();

    /// @brief 初始化FTP
    void initFtp();

    /// @brief 保存按钮点击事件
    void saveBtnClicked();

    /// @brief 上传个人化数据按钮点击事件
    void uploadPrdBtnClicked();

    /// @brief 上传临时存放按钮点击事件
    void uploadTempBtnClicked();

    void dragEnterEvent(QDragEnterEvent *event);

    void dropEvent(QDropEvent *event);

    bool isOrder();

    void buttonDisabled(bool disabled);

    void showInfo();

    void next();

  private:
    Ui_MainWindow           *ui_;
    zel::utility::IniFile ini_;
    zel::ftp::FtpClient  *ftp_;

    Path           *path_;
    OrderInfo      *order_info_;
    PersonDataInfo *person_data_info_;
    ScriptInfo     *script_info_;
};