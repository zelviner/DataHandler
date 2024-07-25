#include "order_processing.h"

#include "utils/utils.h"

#include <memory>
#include <zel/filesystem.h>
using namespace zel::filesystem;

#include <zel/crypto.h>
using namespace zel::crypto;

#include <qdebug>
#include <qdir>
#include <qfile>
#include <qstringlist>
#include <iostream>

OrderProcessing::OrderProcessing(std::shared_ptr<Path> path)
    : path_(path) {}

OrderProcessing::~OrderProcessing() {}

bool OrderProcessing::preProcessing(const std::string &confirm_datagram_dir) {
    File datagram_file(path_->datagram);
    path_->directory       = datagram_file.dirPath();
    auto datagram_zip_path = FilePath::join(datagram_file.dirPath(), datagram_file.prefix());

    // GPG解密数据包
    try {
        Gpg gpg("libgpgme-11.dll");
        gpg.decryptFile(datagram_file.path(), datagram_zip_path);
        std::cout << "File decrypted successfully." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    // 解压数据包
    if (!decompressionZipFile(QString(datagram_zip_path.c_str()), QString(datagram_file.dirPath().c_str()))) {
        return false;
    }
    printf("解压成功\n");

    // 判断订单号是否修改
    int pos = datagram_file.name().find_last_of(".zip.pgp");
    if (pos == std::string::npos) return false;
    auto datagram_dir = datagram_file.name().substr(0, pos - 7);
    if (datagram_dir != confirm_datagram_dir) {
        printf("修改订单");
    }

    return true;
}

bool OrderProcessing::modificateProjectNumber() { printf("修改订单号"); }

bool OrderProcessing::authenticationDir(QString data_filename, QString header, QString data) {

    // // 创建鉴权文件夹
    // createFolder(path_->authenticationPath());

    // // 将首条个人化数据文件移动到鉴权文件夹
    // if (!dataToAuthDir(data_filename, header, data)) return false;

    // // 总部脚本文件夹拷贝到鉴权文件夹下
    // if (!copyFolder(path_->zhScriptPath(), path_->scriptPath(), true)) {
    //     return false;
    // }

    return true;
}

bool OrderProcessing::screenshotDir() {

    // 创建截图文件夹
    Directory screenshot_dir(path_->screenshot);
    if (!screenshot_dir.exists()) {
        screenshot_dir.create();
    }

    return true;
};

bool OrderProcessing::printDir() {

    // 创建打印文件夹
    Directory print_dir(path_->print);
    if (!print_dir.exists()) {
        print_dir.create();
    }

    // 拷贝需要打印的文件
    auto walkFunc = [=](std::string relative_path, Directory dir, File file) -> bool {
        if (file.exists()) {
            if (file.extension() == ".xlsx" || file.extension() == ".xls") {
                if (!file.copy(FilePath::join(path_->print, file.name()))) {
                    return false;
                }
            }
        }
        return true;
    };

    return FilePath::walk(path_->order, walkFunc);
}

bool OrderProcessing::tagDataDir() {

    // 压缩文件
    if (!compressionZipFile(QString(path_->tag_data.c_str()))) return false;

    // 剪切标签数据压缩包
    File tag_data_file(path_->tag_data + ".zip");
    if (!tag_data_file.move(FilePath::join(path_->temp, tag_data_file.name()))) {
        return false;
    }

    return true;
}

bool OrderProcessing::dataToAuthDir(QString data_filename, QString header, QString data) {

    // QFile file(path_->authenticationPath() + "/" + data_filename);
    // if (file.open(QIODevice::WriteOnly)) {
    //     header += "\r\n";
    //     data += "\r\n";
    //     file.write(header.toStdString().c_str());
    //     file.write(data.toStdString().c_str());
    // } else {
    //     qDebug() << "open file error";
    // }

    // file.close();

    return true;
}