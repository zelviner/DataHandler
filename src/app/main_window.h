#pragma once

#include "ui_main_window.h"
#include "order/order.h"
#include "order/person_data.h"
#include "order/script.h"
#include "order_window.h"
#include "loading.h"
#include "myorm/database.h"

#include <zel/zel.h>
#include <memory>
#include <qmainwindow>
#include <qtranslator>

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QMainWindow *parent = nullptr);
    ~MainWindow();

    /// @brief 中文语言切换
    void chineseLanguageAction();

    /// @brief 英文语言切换
    void englishLanguageAction();

    /// @brief 拖拽释放事件
    void dropEvent(QDropEvent *event);

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

    /// @brief 选择金融数据分配表生成路径按钮点击事件
    void selectFinanceGeneratePathBtnClicked();

    /// @brief 选择电信数据分配表生成路径按钮点击事件
    void selectTelecomGeneratePathBtnClicked();

    /// @brief 生成金融数据分配表点击事件
    void generatingFinanceRecordBtnClicked();

    /// @brief 生成电信数据分配表点击事件
    void generatingTelecomRecordBtnClicked();

    /// @brief 选择金融模板文件径按钮点击事件
    void selectFinanceTemplatePathBtnClicked();

    /// @brief 选择电信模板文件径按钮点击事件
    void selectTelecomTemplatePathBtnClicked();

    /// @brief 保存按钮点击事件
    void saveBtnClicked();

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

    void generatingRecordFailure();
    void generatingRecordSuccess();

    void resetCardFailure(const QString &err_msg);
    void resetCardSuccess(const QString &atr);

  private:
    /// @brief 初始化窗口
    void initWindow();

    /// @brief 初始化UI
    void initUI();

    /// @brief 初始化信号槽
    void initSignalSlot();

    /// @brief 初始化配置
    void initConfig(const std::string &config_file);

    /// @brief 初始化日志器
    void initLogger(const std::string &log_file);

    /// @brief 初始化读卡器
    void initCardReader();

    /// @brief 初始化数据库
    void initDatabase();

    void dragEnterEvent(QDragEnterEvent *event);

    void buttonDisabled(bool disabled);

    void showInfo();

    void switchLanguage(const QString &language_file);

    void retranslateUi();

  private:
    Ui_MainWindow *ui_;           // UI界面
    OrderWindow   *order_window_; // 确认订单窗口
    Loading       *loading_;      // 加载窗口
    QTranslator    translator_;   // 翻译器
    QString        current_lang_ = "zh_CN";

    zel::utility::IniFile                 ini_;              // 配置文件
    std::shared_ptr<Path>                 path_;             // 路径
    std::shared_ptr<OrderInfo>            order_info_;       // 订单信息
    std::shared_ptr<PersonDataInfo>       person_data_info_; // 个人化信息
    std::shared_ptr<ScriptInfo>           script_info_;      // 脚本信息
    std::shared_ptr<zel::myorm::Database> finance_db_;       // 金融数据库
    std::shared_ptr<zel::myorm::Database> telecom_db_;       // 电信数据库
};