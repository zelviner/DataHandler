#pragma once

#include "ui_main_window.h"
#include "order-info/order.h"
#include "order-info/person_data.h"
#include "order-info/script.h"
#include "ftp_loading.h"
#include "order_window.h"
#include "order-processing/order_processing.h"

#include <zel/ftp.h>
#include <zel/utility.h>
#include <xhlanguage/card_reader.h>
#include <memory>
#include <qmainwindow>

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QMainWindow *parent = nullptr);
    ~MainWindow();

    /// @brief 拖拽释放事件
    void dropEvent(QDropEvent *event);

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

  private:
    /// @brief 初始化窗口
    void initWindow();

    /// @brief 初始化UI
    void initUI();

    /// @brief 初始化信号槽
    void initSignalSlot();

    /// @brief 初始化配置
    void initConfig();

    /// @brief 获取订单全部信息
    bool orderInfo(QString &error);

    /// @brief 订单处理
    bool orderProcessing(QString &error);

    /// @brief 上传文件到FTP
    /// @param local_file_path  本地文件路径
    /// @param remote_file_path  远程文件路径
    bool uploadFile2FTP(const std::string &local_file_path, const std::string &remote_file_path);

    void dragEnterEvent(QDragEnterEvent *event);

    /// @brief 是否是订单
    bool isOrder();

    void buttonDisabled(bool disabled);

    void showInfo();

    void next();

  public slots:
    void confirmOrder(const QString &confirm_datagram_dir);
    void cancelOrder();

    void bareAtr(const QString &bare_atr);
    void whiteAtr(const QString &white_atr);
    void finishedAtr(const QString &finished_atr);

    void failure(const QString &err_type, const QString &err_msg);
    void success();

  private:
    Ui_MainWindow                   *ui_;               // UI界面
    OrderWindow                     *order_window_;     // 确认订单窗口
    zel::utility::IniFile            ini_;              // 配置文件
    std::shared_ptr<Path>            path_;             // 路径
    std::unique_ptr<OrderProcessing> order_processing_; // 订单处理
    std::shared_ptr<OrderInfo>       order_info_;       // 订单信息
    PersonDataInfo                  *person_data_info_; // 个人化信息
    ScriptInfo                      *script_info_;      // 脚本信息
    std::shared_ptr<CardReader>      card_reader_;      // 读卡器

    FtpLoading *ftp_loading_;
};