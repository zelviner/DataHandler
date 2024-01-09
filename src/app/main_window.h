#pragma once

#include "card-reader/card_reader.hpp"
#include "info/order.h"
#include "info/person_data.h"
#include "info/script.h"
#include "ui_main_window.h"

#include <QMainWindow>
#include <memory>
#include <utility/ini_file.h>

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

    /// @brief 保存按钮点击事件
    void saveBtnClicked();

    /// @brief 打开预个人化脚本按钮点击事件
    void openPersonalBtnClicked();

    /// @brief 打开后个人化脚本按钮点击事件
    void openPostPersonalBtnClicked();

    /// @brief 打开检测脚本按钮点击事件
    void openCheckBtnClicked();

    /// @brief 打开清卡脚本按钮点击事件
    void openClearCardBtnClicked();

    /// @brief 卡片复位按钮点击事件
    void resetCardBtnClicked();

    /// @brief 写卡按钮点击事件
    void writeCardBtnClicked();

    /// @brief 清卡按钮点击事件
    void clearCardBtnClicked();

    /// @brief 上传个人化数据按钮点击事件
    void uploadPrdBtnClicked();

    /// @brief 上传临时存放按钮点击事件
    void uploadTempBtnClicked();

    void uploadFile2FTP(const std::string &local_file_path, const std::string &remote_file_path);

    void dragEnterEvent(QDragEnterEvent *event);

    void dropEvent(QDropEvent *event);

    bool isOrder();

    void buttonDisabled(bool disabled);

    void showInfo();

    void next();

  public slots:
    void bareAtr(const QString &bare_atr);
    void whiteAtr(const QString &white_atr);
    void finishedAtr(const QString &finished_atr);

  private:
    Ui_MainWindow        *ui_;
    zel::utility::IniFile ini_;
    Path                 *path_;
    OrderInfo            *order_info_;
    PersonDataInfo       *person_data_info_;
    ScriptInfo           *script_info_;

    std::shared_ptr<CardReader> card_reader_;
};