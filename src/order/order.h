#pragma once

#include "path.h"
#include "order_parser.h"
#include "person_data.h"
#include "script.h"

#include <memory>
#include <qstring>

class Order {

  public:
    Order(std::shared_ptr<Path> path);
    ~Order();

    /// @brief 预处理
    bool preProcessing();

    /// @brief 处理
    bool processing();

    /// @brief 显示路径
    void showPath();

    std::shared_ptr<OrderInfo> orderInfo();

    std::shared_ptr<PersonDataInfo> personDataInfo();

    std::shared_ptr<ScriptInfo> scriptInfo();

  private:
    std::shared_ptr<Path>           path_;
    std::shared_ptr<OrderInfo>      order_info_;
    std::shared_ptr<PersonDataInfo> person_data_info_;
    std::shared_ptr<ScriptInfo>     script_info_;
};