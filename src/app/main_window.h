#pragma once

#include <xhlanguage/xhlanguage.h>
#include "ui_main_window.h"
#include "order/order.h"
#include "order/person_data.h"
#include "order/script.h"
#include "order_window.h"
#include "loading.h"

#include <zel/zel.h>
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

  public slots:
    void confirmOrder(const std::string &confirm_datagram_dir_name);
    void cancelOrder();

    void bareAtr(const QString &bare_atr);
    void whiteAtr(const QString &white_atr);
    void finishedAtr(const QString &finished_atr);

    void uploadFileFailure(const QString &err_type, const QString &err_msg);
    void uploadFileSuccess();

    void handleOrderFailure(const QString &err_msg);
    void handleOrderSuccess(std::shared_ptr<OrderInfo> order_info, std::shared_ptr<PersonDataInfo> person_data_info, std::shared_ptr<ScriptInfo> script_info);

  private:
    /// @brief 初始化窗口
    void initWindow();

    /// @brief 初始化UI
    void initUI();

    /// @brief 初始化信号槽
    void initSignalSlot();

    /// @brief 初始化配置
    void initConfig();

    /// @brief 初始化日志器
    void initLogger();

    void dragEnterEvent(QDragEnterEvent *event);

    void buttonDisabled(bool disabled);

    void showInfo();

  private:
    Ui_MainWindow *ui_;           // UI界面
    OrderWindow   *order_window_; // 确认订单窗口
    Loading       *loading_;      // 加载窗口

    zel::utility::IniFile           ini_;              // 配置文件
    std::shared_ptr<Path>           path_;             // 路径
    std::shared_ptr<OrderInfo>      order_info_;       // 订单信息
    std::shared_ptr<PersonDataInfo> person_data_info_; // 个人化信息
    std::shared_ptr<ScriptInfo>     script_info_;      // 脚本信息
    std::shared_ptr<CardReader>     card_reader_;      // 读卡器
};