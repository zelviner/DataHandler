#include "do_order.h"

#include "public/qt-utility/qt_utility.h"
using namespace zel::qtutility;

#include "public/filesystem/directory.h"
#include "public/filesystem/file.h"
using namespace zel::filesystem;

#include "public/utility/string.h"
using namespace zel::utility;

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStringList>

DoOrder::DoOrder(Path *path)
    : path_(path) {}

DoOrder::~DoOrder() {}

bool DoOrder::authenticationDir(QString data_filename, QString header, QString data) {

    createFolder(path_->authenticationPath());

    if (!dataToAuthDir(data_filename, header, data)) return false;

    if (!scriptToAuthDir()) return false;

    return true;
}

bool DoOrder::screenshotDir(QString filename) {

    createFolder(path_->screenshotPath());

    QFile file(path_->screenshotPath() + "/" + filename);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.close();

    return true;
};

bool DoOrder::printDir() {
    createFolder(path_->printPath());

    // 拷贝需要打印的文件
    QString filename;
    for (auto print_path : path_->printsPath()) {
        filename = print_path.mid(print_path.lastIndexOf("/") + 1);
        if (!copyFile(print_path, path_->printPath() + "/" + filename, true)) return false;
    }

    return true;
}

bool DoOrder::tagDataDir() {
    createFolder(path_->tagDataPath());

    // 拷贝标签数据
    copyFolder(path_->zhTagDataPath(), path_->tagDataPath(), true);

    // 压缩文件
    if (!compressionZipFile(path_->tagDataPath())) return false;

    // 删除原文件
    if (!deleteFileOrFolder(path_->tagDataPath())) return false;

    return true;
}

bool DoOrder::dataToAuthDir(QString data_filename, QString header, QString data) {
    QFile file(path_->authenticationPath() + "/" + data_filename);
    if (file.open(QIODevice::WriteOnly)) {
        header += "\r\n";
        data += "\r\n";
        file.write(header.toStdString().c_str());
        file.write(data.toStdString().c_str());
    } else {
        qDebug() << "open file error";
    }

    file.close();

    return true;
}

bool DoOrder::scriptToAuthDir() {
    auto filename    = path_->scriptPath().mid(path_->scriptPath().lastIndexOf("/", -2));
    auto target_path = path_->authenticationPath() + "/" + filename;

    if (!copyFolder(path_->scriptPath(), target_path, true)) {
        qDebug() << "copy folder error";
        return false;
    }
    return true;
}

bool DoOrder::clearScriptDir(QString filename, QString clear_script, QString atr3) {

    createFolder(path_->clearCardPath());

    QFile file(path_->clearCardPath() + "/" + filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(" [DATA]\r\nE_DataFormat=ds.NUMBER:20,ds.CRC:0\r\nMD5=\r\nATR3=");
        file.write((atr3 + "\r\n").toStdString().c_str());
        file.write(".START=1\r\n");
        file.write(clear_script.toStdString().c_str());

        file.close();
        return true;
    }

    return false;
}
