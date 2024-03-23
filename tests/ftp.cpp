#include <iostream>
#include <vector>

#include <ftp/ftp.h>
using namespace zel::ftp;

int main() {

    FtpClient ftp("172.22.173.36", "datacenter", "CQ!Qw2!Qw2");

    if (!ftp.connect()) {
        std::cout << "FTP 连接失败!!!" << std::endl;
        return -1;
    }

    if (!ftp.login()) {
        std::cout << "FTP 登录失败!!!" << std::endl;
        return -1;
    }

    // auto list = ftp.listDir("../test/你好.txt", "/data/ftp/output");

    if (!ftp.uploadFile("../test/你好.txt", "/data/ftp/output/ftplist.txt")) {
        std::cout << "文件上传失败" << std::endl;
    }

    return 0;
}
