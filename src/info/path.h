#pragma once

#include <QDebug>
#include <QString>
#include <QStringList>

/// @brief 路径结构体
struct RelativePath {
    QString order_path;          // 订单文件夹路径
    QString data_path;           // 个人化数据文件夹路径
    QString ftp_data_path;       // 要上传到FTP服务器的个人化数据文件夹路径
    QString script_path;         // 脚本包文件夹路径
    QString screenshot_path;     // 截图文件夹路径
    QString authentication_path; // 鉴权文件夹路径
    QString temp_path;           // 临时存放文件夹路径
    QString src_tag_data_path;   // 总部标签数据文件夹路径
    QString dest_tag_data_path;  // 上传标签数据文件夹路径
    QString clear_card_path;     // 清卡脚本文件路径

    QString     print_path;  // 打印文件夹路径
    QStringList print_paths; // 打印文件路径列表
};

class Path {

  public:
    Path(QString dir_path);
    ~Path();

    /// @brief 显示路径
    void show();

    QString     dirPath();
    QString     orderPath();
    QString     dataPath();
    QString     ftpDataPath();
    QString     scriptPath();
    QString     screenshotPath();
    QString     authenticationPath();
    QString     tempPath();
    QString     srcTagDataPath();
    QString     destTagDataPath();
    QString     clearCardPath();
    QString     printPath();
    QStringList printsPath();


    void dirPath(QString path);
    void orderPath(QString path);
    void dataPath(QString path);
    void ftpDataPath(QString path);
    void scriptPath(QString path);
    void screenshotPath(QString path);
    void authenticationPath(QString path);
    void tempPath(QString path);
    void srcTagDataPath(QString path);
    void destTagDataPath(QString path);
    void clearCardPath(QString path);
    void printPath(QString path);
    void printsPath(QStringList path_list);

  private:
    QString abslutePath(QString path);

  private:
    QString      dir_path_;
    RelativePath path_;
};