#include "ftp.h"
#include "ftplib.h"

#include "../utility/string.h"
using namespace zel::utility;

namespace zel {
namespace ftp {

FtpClient::FtpClient(const std::string &host, const std::string &user, const std::string &password, int port)
    : host_(host)
    , user_(user)
    , password_(password)
    , port_(port)
    , ftp_(new ftplib()) {}

FtpClient::~FtpClient() { disconnect(); }

bool FtpClient::connect() {
    if (host_.empty() || user_.empty() || password_.empty()) return false;
    return ftp_->Connect(host_.c_str());
}

void FtpClient::disconnect() {
    if (ftp_) {
        ftp_->Quit();
        delete ftp_;
        ftp_ = nullptr;
    }
}

bool FtpClient::login() { return ftp_->Login(user_.c_str(), password_.c_str()); }

bool FtpClient::listDir(const std::string &remote_dir_path, std::vector<std::string> &file_list) {
    std::string file_name;

    if (ftp_->Dir(file_name.c_str(), remote_dir_path.c_str()) == 0) {
        ftp_->Pwd((char *) file_name.c_str(), 256);
        printf("%s\n", file_name.c_str());
        return false;
    }

    printf("%s\n", file_name.c_str());

    return true;
}

bool FtpClient::uploadFile(const std::string &local_file_path, const std::string &remote_file_path) {
    zel::filesystem::Directory dir(local_file_path);

    // 判断是否为文件夹
    if (dir.exists()) {
        // 创建远程文件夹
        ftp_->Mkdir(remote_file_path.c_str());

        // 获取本地文件夹下的所有文件
        auto file_list = dir.files();

        // 递归上传文件
        for (auto file : file_list) {
            if (!uploadFile(local_file_path + "/" + file.name(), remote_file_path + "/" + file.name())) {
                return false;
            }
        }
    } else {
        // 上传文件
        if (ftp_->Put(local_file_path.c_str(), remote_file_path.c_str(), ftplib::image) == 0) {
            return false;
        }
    }

    return true;
}

bool FtpClient::downloadFile(const std::string &remote_file_path, const std::string &local_file_path) {

    // 兼容中文路径
    auto local_file_path_utf8  = String::toUtf8(local_file_path);
    auto remote_file_path_utf8 = String::toUtf8(remote_file_path);

    if (ftp_->Get(local_file_path.c_str(), remote_file_path.c_str(), ftplib::ascii) == 0) {
        return false;
    }

    return true;
}

} // namespace ftp
} // namespace zel
