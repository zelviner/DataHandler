#pragma once

#include "path.h"
#include "person_data.h"
#include "script.h"

#include <memory>
#include <qstring>

/// @brief 订单信息结构体
struct OrderInfo {
    std::string order_dir_name; // 订单文件夹名称
    std::string project_number; // 工程单号
    std::string order_number;   // 订单号
    std::string project_name;   // 项目名称
    std::string chip_model;     // 卡片型号
    std::string rf_code;        // 需求编码
    std::string script_package; // 脚本包
};

class Order {

  public:
    Order(std::shared_ptr<Path> path);
    ~Order();

    /// @brief 预处理
    bool preProcessing();

    /// @brief 修改
    bool modify();

    /// @brief 处理
    bool processing();

    /// @brief 显示路径
    void showPath();

    std::shared_ptr<OrderInfo>      orderInfo();
    std::shared_ptr<PersonDataInfo> personDataInfo();
    std::shared_ptr<ScriptInfo>     scriptInfo();

  private:
    /// @brief 截图文件夹
    bool screenshotDir();

    /// @brief 打印文件夹
    bool printDir();

    /// @brief 标签数据文件夹
    bool tagDataDir();

    std::shared_ptr<OrderInfo> orderInfo(const std::string &order_dir_name);

  private:
    std::shared_ptr<Path>           path_;
    std::shared_ptr<OrderInfo>      order_info_;
    std::shared_ptr<PersonDataInfo> person_data_info_;
    std::shared_ptr<ScriptInfo>     script_info_;
};