#pragma once

#include "authenticator.h"
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

    /// @brief 工具栏操作
    void chineseLanguageAction(); // 中文语言切换
    void englishLanguageAction(); // 英文语言切换

    /// @brief 拖拽释放事件
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);

    /// @brief 信息 Tab
    void openPersonalBtnClicked();     // 打开预个人化脚本按钮点击事件
    void openPostPersonalBtnClicked(); // 打开后个人化脚本按钮点击事件
    void openCheckBtnClicked();        // 打开检测脚本按钮点击事件
    void openClearCardBtnClicked();    // 打开清卡脚本按钮点击事件

    /// @brief 鉴权 Tab
    void resetCardBtnClicked(); // 卡片复位按钮点击事件
    void writeCardBtnClicked(); // 写卡按钮点击事件
    void clearCardBtnClicked(); // 清卡按钮点击事件

    /// @brief 上传 Tab
    void uploadPrdBtnClicked();  // 上传个人化数据按钮点击事件
    void uploadTempBtnClicked(); // 上传临时存放按钮点击事件

    /// @brief 制表 Tab
    void selectFinanceGeneratePathBtnClicked(); // 选择金融数据分配表生成路径按钮点击事件
    void generatingFinanceRecordBtnClicked();   // 生成金融数据分配表点击事件
    void deleteTelecomOrderBtnClicked();        // 删除电信订单按钮点击事件
    void selectTelecomGeneratePathBtnClicked(); // 选择电信数据分配表生成路径按钮点击事件
    void generatingTelecomRecordBtnClicked();   // 生成电信数据分配表点击事件

    /// @brief 配置 Tab
    void selectFinanceTemplatePathBtnClicked(); // 选择金融模板文件径按钮点击事件
    void selectTelecomTemplatePathBtnClicked(); // 选择电信模板文件径按钮点击事件
    void saveBtnClicked();                      // 保存按钮点击事件

  public slots:
    void confirmOrder(const std::string &confirm_datagram_dir_name);
    void cancelOrder();

    void confirmDeleteOrder(const std::string &password);
    void cancelDeleteOrder();

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
    void init_window();

    /// @brief 初始化UI
    void init_ui();

    /// @brief 初始化信号槽
    void init_signal_slot();

    /// @brief 初始化配置
    void init_config(const std::string &config_file);

    /// @brief 初始化日志器
    void init_logger(const std::string &log_file);

    /// @brief 初始化读卡器
    void init_card_reader();

    /// @brief 初始化数据库
    void init_database();

    void button_disabled(bool disabled);

    void show_info();

    void switch_language(const QString &language_file);

    void retranslate_ui();

  private:
    Ui_MainWindow *ui_;            // UI界面
    OrderWindow   *order_window_;  // 确认订单窗口
    Loading       *loading_;       // 加载窗口
    Authenticator *authenticator_; // 身份验证器
    QTranslator    translator_;    // 翻译器
    QString        current_lang_ = "zh_CN";

    zel::utility::Ini                     ini_;              // 配置文件
    std::shared_ptr<Path>                 path_;             // 路径
    std::shared_ptr<OrderInfo>            order_info_;       // 订单信息
    std::shared_ptr<PersonDataInfo>       person_data_info_; // 个人化信息
    std::shared_ptr<ScriptInfo>           script_info_;      // 脚本信息
    std::shared_ptr<zel::myorm::Database> finance_db_;       // 金融数据库
    std::shared_ptr<zel::myorm::Database> telecom_db_;       // 电信数据库
};