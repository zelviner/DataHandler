#pragma once

// #include "public/utility/ini_file.h"
// #include "public/utility/string.h"
// using namespace zel::utility;

// #include "public/ftp/ftp.h"
// using namespace zel::ftp;

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

// 自定义的工作线程类
class UploadPrd : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, LOGIN, UPLOAD, FINISH };

    UploadPrd() {}

    // void ini(const IniFile &ini) { ini_ = ini; }
    void localPrdPath(const std::string &local_prd_path) { local_prd_path_ = local_prd_path; }
    void remotePrdPath(const std::string &remote_prd_path) { remote_prd_path_ = remote_prd_path; }

    // 重写run函数，在这里执行线程的工作
    void run() override {

        // std::string host     = ini_["ftp"]["host"];
        // int         port     = ini_["ftp"]["port"];
        // std::string username = ini_["ftp"]["username"];
        // std::string password = ini_["ftp"]["password"];

        // FtpClient ftp(host, username, password, port);
        // if (!ftp.connect()) {
        //     emit failure(CONNECT, "FTP连接失败, 请检查FTP服务器IP和端口是否正确");
        //     return;
        // }

        // if (!ftp.login()) {
        //     emit failure(LOGIN, "登录失败, 请检查用户名和密码是否正确");
        //     return;
        // }

        // if (!ftp.uploadFile(local_prd_path_, remote_prd_path_)) {
        //     ftp.disconnect();
        //     emit failure(UPLOAD, "上传失败, 请检查远程路径是否正确");
        //     return;
        // }
    }

  signals:
    // 信号函数，用于向外界发射信号
    void failure(UploadPrd::Type type, const QString &err_msg);

  private:
    // IniFile     ini_;
    std::string local_prd_path_;
    std::string remote_prd_path_;
};