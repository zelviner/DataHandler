#include <gtest/gtest.h>
#include <zel/ftp.h>
using namespace zel::ftp;

#include <zel/utility.h>
using namespace zel::utility;

#include <zel/filesystem.h>
using namespace zel::filesystem;

#include <codecvt>
#include <locale>
#include <stdarg.h>
#include <stdio.h>
#include <string>

std::string current_path = "D:/Workspaces/C++/Visual Studio Code/zeltest/test/ftp";

FtpClient ftp("172.22.173.36", "datacenter", "1234");

TEST(Ftp, connect) {

    if (ftp.connect()) {
        printf("连接成功!!!\n");
    } else {
        printf("连接失败!\n");
    }

    if (ftp.login()) {
        printf("登录成功!\n");
    } else {
        printf("登录失败!\n");
    }

    ftp.disconnect();

    SUCCEED() << "测试成功";
}

TEST(Ftp, list_dir) {

    if (!ftp.connect()) {
        FAIL() << "连接失败";
    }

    if (!ftp.login()) {
        FAIL() << "登录失败";
    }

    auto files = ftp.listDir("/data/ftp/测试上传功能");

    printf("%s\n", files.c_str());

    ftp.disconnect();

    SUCCEED() << "测试成功";
}

TEST(Ftp, upload_file) {

    if (!ftp.connect()) {
        FAIL() << "连接失败!";
    }

    if (!ftp.login()) {
        FAIL() << "登录失败!";
    }

    if (!ftp.uploadFile(current_path + "/test-upload/a.txt", "/data/ftp/测试上传功能/a.txt")) {
        FAIL() << "上传文件失败" << current_path + "/test-upload/a.txt";
    }

    // 
    std::wstring wpath = L"D:/Workspaces/C++/Visual Studio Code/zeltest/test/ftp/测试上传目录/测试.txt";
    std::string  path  = String::wstring2Utf8(wpath);
    if (!ftp.uploadFile(path, "/data/ftp/测试上传功能")) {
        FAIL() << "上传文件失败: " << path;
    }

    ftp.disconnect();

    SUCCEED() << "上传成功";
}

TEST(Ftp, upload_dir) {

    if (!ftp.connect()) {
        FAIL() << "连接失败!";
    }

    if (!ftp.login()) {
        FAIL() << "登录失败!";
    }

    if (!ftp.uploadFile(current_path + "/test-upload", "/data/ftp/测试上传功能/test-uplaod")) {
        FAIL() << "上传文件失败";
    }

    ftp.disconnect();

    SUCCEED() << "上传成功";
}