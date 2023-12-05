#pragma once

#include <QString>

#include "info/path.h"

class DoOrder {

  public:
    DoOrder(Path *path);
    ~DoOrder();

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
    Path *path_;
};