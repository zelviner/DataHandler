#pragma once

#include "utils/utils.h"

#include <qcoreapplication>
#include <qthread>
#include <zel/core.h>

// 自定义的工作线程类
class UploadFile : public QThread {
    Q_OBJECT

  public:
    UploadFile(const zel::utility::Ini &ini, const std::string &local_path, const std::string &remote_path, bool is_temp)
        : ini_(ini)
        , local_path_(local_path)
        , remote_path_(remote_path)
        , is_temp_(is_temp) {}

  signals:
    // 信号函数，用于向外界发射信号
    void failure(const QString &err_type, const QString &err_msg);
    void success();

  protected:
    void run() override {

        std::string host     = ini_["ftp"]["host"];
        std::string username = ini_["ftp"]["username"];
        std::string password = ini_["ftp"]["password"];
        std::string remote   = "ftp://" + host + remote_path_;
        if (!Utils::ftpUploadDir(local_path_, remote, username + ":" + password)) {
            emit failure("上传失败", "请检查远程路径是否正确");
            return;
        }

        emit success();
    }

  private:
    zel::utility::Ini ini_;
    std::string       local_path_;
    std::string       remote_path_;
    bool              is_temp_;
};