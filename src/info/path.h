#pragma once

#include <qdebug>
#include <qstring>
#include <qstringlist>

/// @brief 路径结构体
struct RelativePath {
    QString     zh_order_path;       // 珠海总部--订单文件夹路径
    QString     zh_data_path;        // 珠海总部--个人化数据文件夹相对路径
    QString     zh_script_path;      // 珠海总部--脚本包文件夹相对路径
    QString     zh_tag_data_path;    // 珠海总部--标签数据文件夹相对路径
    QStringList zh_print_paths;      // 珠海总部--待打印文件相对路径列表
    QString     data_path;           // 个人化数据文件夹相对路径
    QString     script_path;         // 脚本包文件夹相对路径
    QString     screenshot_path;     // 截图文件夹相对路径
    QString     authentication_path; // 鉴权文件夹相对路径
    QString     temp_path;           // 临时存放文件夹相对路径
    QString     tag_data_path;       // 标签数据文件夹相对路径
    QString     clear_card_path;     // 清卡脚本文件相对路径
    QString     print_path;          // 打印文件夹相对路径
};

class Path {

  public:
    Path(QString dir_path);
    ~Path();

    /// @brief 显示路径
    void show();

    QString     dirPath();
    QString     zhOrderPath();
    QString     zhDataPath();
    QString     zhScriptPath();
    QString     zhTagDataPath();
    QStringList zhPrintPaths();
    QString     dataPath();
    QString     scriptPath();
    QString     screenshotPath();
    QString     authenticationPath();
    QString     tempPath();
    QString     tagDataPath();
    QString     clearCardPath();
    QString     printPath();

    void dirPath(QString path);
    void zhOrderPath(QString path);
    void zhDataPath(QString path);
    void zhScriptPath(QString path);
    void zhTagDataPath(QString path);
    void zhPrintPaths(QStringList path_list);
    void dataPath(QString path);
    void scriptPath(QString path);
    void screenshotPath(QString path);
    void authenticationPath(QString path);
    void tempPath(QString path);
    void tagDataPath(QString path);
    void clearCardPath(QString path);
    void printPath(QString path);

  private:
    QString abslutePath(QString path);

  private:
    QString      dir_path_;
    RelativePath path_;
};