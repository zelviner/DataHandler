#pragma once

#include <qdebug>
#include <qstring>
#include <qstringlist>
#include <vector>

/// @brief 路径结构体
struct RelativePath {
    std::string              data_path;           // 个人化数据文件夹相对路径
    std::string              script_path;         // 脚本包文件夹相对路径
    std::string              screenshot_path;     // 截图文件夹相对路径
    std::string              temp_path;           // 临时存放文件夹相对路径
    std::string              tag_data_path;       // 标签数据文件夹相对路径
    std::string              print_path;          // 打印文件夹相对路径
    std::vector<std::string> print_paths;         // 待打印文件相对路径列表
};

class Path {

  public:
    Path(std::string datagram_path);
    ~Path();

    /// @brief 显示路径
    void show();

    std::string              datagramPath();
    std::string              dirPath();
    std::string              dataPath();
    std::string              scriptPath();
    std::string              screenshotPath();
    std::string              tempPath();
    std::string              tagDataPath();
    std::string              printPath();
    std::vector<std::string> printPaths();

    void dirPath(std::string path);
    void dataPath(std::string path);
    void scriptPath(std::string path);
    void screenshotPath(std::string path);
    void tempPath(std::string path);
    void tagDataPath(std::string path);
    void printPath(std::string path);
    void printPaths(std::vector<std::string> path_list);

  private:
    std::string abslutePath(std::string path);

  private:
    std::string  datagram_path_;
    std::string  dir_path_;
    RelativePath path_;
};