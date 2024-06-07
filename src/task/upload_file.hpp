#pragma once

#include <zel/ftp.h>
#include <zel/utility.h>
#include <qcoreapplication>
#include <qdebug>
#include <qthread>

// 自定义的工作线程类
class UploadFile : public QThread {
    Q_OBJECT

  public:
    UploadFile() {}

    void ini(const zel::utility::IniFile &ini) { ini_ = ini; }

    void localFilePath(const std::string &local_file_path) { local_file_path_ = local_file_path; }

    void remoteFilePath(const std::string &remote_file_path) { remote_file_path_ = remote_file_path; }

    // 重写run函数，在这里执行线程的工作
    void run() override {

        std::string host     = ini_["ftp"]["host"];
        int         port     = ini_["ftp"]["port"];
        std::string username = ini_["ftp"]["username"];
        std::string password = ini_["ftp"]["password"];

        zel::ftp::FtpClient ftp(host, username, password, port);
        if (!ftp.connect()) {
            emit failure("服务器连接失败", "请检查FTP服务器IP和端口是否正确");
            return;
        }

        if (!ftp.login()) {
            emit failure("FTP登录失败", "请检查用户名和密码是否正确");
            return;
        }

        printf("%s -------- %s\n", local_file_path_.c_str(), remote_file_path_.c_str());
        if (!ftp.uploadFile(local_file_path_, remote_file_path_)) {
            ftp.disconnect();
            emit failure("上传失败", "请检查远程路径是否正确");
            return;
        }

        ftp.disconnect();
        emit success();
    }

  signals:
    // 信号函数，用于向外界发射信号
    void failure(const QString &err_type, const QString &err_msg);
    void success();

  private:
    zel::utility::IniFile ini_;
    std::string           local_file_path_;
    std::string           remote_file_path_;
};