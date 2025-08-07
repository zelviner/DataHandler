#pragma once

#include "utils/utils.h"
#include "order/order.h"
#include "order/path.h"

#include <zel/zel.h>
#include <memory>
#include <qcoreapplication>
#include <qthread>

// 自定义的工作线程类
class UploadFile : public QThread {
    Q_OBJECT

  public:
    UploadFile(const zel::utility::IniFile &ini, const std::string &local_path, const std::string &remote_path, bool is_temp, std::shared_ptr<Path> path)
        : ini_(ini)
        , local_path_(local_path)
        , remote_path_(remote_path)
        , is_temp_(is_temp)
        , path_(path) {}

    // 重写run函数，在这里执行线程的工作
    void run() override {

        if (is_temp_) {
            // 压缩截图文件夹
            if (!Utils::compressionZipFile(path_->screenshot, true)) {
                failure("上传失败", "压缩截图文件夹失败");
                return;
            }

            // 压缩临时文件夹
            if (!Utils::compressionZipFile(path_->temp, true)) {
                failure("上传失败", "压缩临时文件夹失败");
                return;
            }

            // 备份订单
            Order order(path_);
            if (!order.backup(ini_["path"]["local_backup_path"])) {
                failure("上传失败", "备份文件夹失败");
                return;
            }
        }

        std::string host     = ini_["ftp"]["host"];
        int         port     = ini_["ftp"]["port"];
        std::string username = ini_["ftp"]["username"];
        std::string password = ini_["ftp"]["password"];

        // zel::ftp::FtpClient ftp(host, username, password, port);
        // if (!ftp.connect()) {
        //     emit failure("服务器连接失败", "请检查FTP服务器IP和端口是否正确");
        //     return;
        // }

        // if (!ftp.login()) {
        //     emit failure("FTP登录失败", "请检查用户名和密码是否正确");
        //     return;
        // }
        std::string remote = "ftp://" + host + remote_path_;
        if (!Utils::ftpUploadDir(local_path_, remote, username + ":" + password)) {
            emit failure("上传失败", "请检查远程路径是否正确");
            return;
        }

        emit success();
    }

  signals:
    // 信号函数，用于向外界发射信号
    void failure(const QString &err_type, const QString &err_msg);
    void success();

  private:
    zel::utility::IniFile ini_;
    std::string           local_path_;
    std::string           remote_path_;
    bool                  is_temp_;
    std::shared_ptr<Path> path_;
};