#pragma once

#include <memory>
#include <qstring>

#include <zel/filesystem.h>

#include "order-info/path.h"

class OrderProcessing {

  public:
    OrderProcessing(std::shared_ptr<Path> path);
    ~OrderProcessing();

    /// @brief 预处理
    bool preProcessing();

    /// @brief 修改工程单号和订单号
    bool modificateProjectNumber();

    /// @brief 鉴权文件夹
    /// @param data_filename 首条个人化数据文件名
    /// @param header 数据头信息
    /// @param data 数据
    bool authenticationDir(QString data_filename, QString header, QString data);

    /// @brief 截图文件夹
    bool screenshotDir(QString filename);

    /// @brief 打印文件夹
    bool printDir();

    /// @brief 标签数据文件夹
    bool tagDataDir();

    /// @brief MD5清卡文件夹
    /// @param filename 文件名
    bool clearScriptDir(QString filename, QString clear_script, QString atr3);

  private:
    /// @brief 将首条个人化数据文件移动到鉴权文件夹
    /// @param data_filename 首条个人化数据文件名
    /// @param header 数据头信息
    /// @param data 数据
    bool dataToAuthDir(QString data_filename, QString header, QString data);

  private:
    std::shared_ptr<Path> path_;
};