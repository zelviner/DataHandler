#pragma once

#include "ftplib.h"
#include <string>
#include <vector>

namespace zel {
namespace ftp {

/// @brief 在ftplibpp库的基础上封装的FTP客户端
class FtpClient {

  public:
    /// @brief 构造函数
    /// @param[in] host FTP服务器地址
    /// @param[in] user FTP用户名
    /// @param[in] password FTP密码
    /// @param[in] port FTP端口号
    FtpClient(const std::string &host, const std::string &user, const std::string &password, int port = 21);

    /// @brief 析构函数
    ~FtpClient();

    /// @brief 连接FTP服务器
    /// @return 已连接返回true，未连接返回false
    bool connect();

    /// @brief 断开FTP服务器
    void disconnect();

    /// @brief 登录
    /// @return 成功返回true，失败返回false
    bool login();

    /// @brief 列出目录下的文件
    /// @param[in] remote_dir_path 远程目录路径
    /// @param[out] file_list 文件列表
    /// @return 成功返回true，失败返回false
    bool listDir(const std::string &remote_dir_path, std::vector<std::string> &file_list);

    /// @brief 上传文件
    /// @param[in] local_file_path 本地文件路径
    /// @param[in] remote_file_path 远程文件路径
    /// @return 成功返回true，失败返回false
    bool uploadFile(const std::string &local_file_path, const std::string &remote_file_path);

    /// @brief 下载文件
    /// @param[in] remote_file_path 远程文件路径
    /// @param[in] local_file_path 本地文件路径
    /// @return 成功返回true，失败返回false
    bool downloadFile(const std::string &remote_file_path, const std::string &local_file_path);

    // /// @brief 删除文件
    // /// @param[in] remote_file_path 远程文件路径
    // /// @return 成功返回true，失败返回false
    // bool deleteFile(const std::string &remote_file_path);

    // /// @brief 创建目录
    // /// @param[in] remote_dir_path 远程目录路径
    // /// @return 成功返回true，失败返回false
    // bool createDir(const std::string &remote_dir_path);

    // /// @brief 删除目录
    // /// @param[in] remote_dir_path 远程目录路径
    // /// @return 成功返回true，失败返回false
    // bool deleteDir(const std::string &remote_dir_path);

    // /// @brief 判断文件是否存在
    // /// @param[in] remote_file_path 远程文件路径
    // /// @return 存在返回true，不存在返回false
    // bool isFileExist(const std::string &remote_file_path);

    // /// @brief 判断目录是否存在
    // /// @param[in] remote_dir_path 远程目录路径
    // /// @return 存在返回true，不存在返回false
    // bool isDirExist(const std::string &remote_dir_path);

  private:
    /// @brief 检查返回值
    /// @param[in] result 返回值
    /// @return 成功返回true，失败返回false
    bool checkResult(int result);

  private:
    ftplib *ftp_; ///< FTP客户端

    std::string host_;     ///< FTP服务器地址
    std::string user_;     ///< FTP用户名
    std::string password_; ///< FTP密码
    int         port_;     ///< FTP端口号
};

} // namespace ftp
} // namespace zel