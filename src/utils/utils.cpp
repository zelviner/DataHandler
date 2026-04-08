#include "utils.h"
#include "QtGui/private/qzipreader_p.h"

#include <qdir>
#include <qfileinfo>
#include <curl/curl.h>
#include <xlnt/xlnt.hpp>
#include <zel/core.h>

Utils::Utils() {}

Utils::~Utils() {}

bool Utils::compressionZipFile(const std::string &file_path, bool remove) {
    size_t pos = file_path.find_last_of("/");
    if (pos == std::string::npos) return false;
    std::string save_path = file_path.substr(0, pos);
    return compressionZipFile(file_path, save_path, remove);
}

bool Utils::compressionZipFile(const std::string &file_path, const std::string &save_path, bool remove) {
    QString qfile_path = QString(file_path.c_str());
    QString qsave_path = QString(save_path.c_str());
    if (qfile_path.isEmpty() || qsave_path.isEmpty()) {
        return false;
    }
    if (!QFile::exists(qfile_path) || !QFileInfo(qsave_path).isDir()) {
        return false;
    }

    // 压缩的是一个文件
    if (QFileInfo(qfile_path).isFile()) {
        QString fileName       = QFileInfo(qfile_path).baseName();
        QString writerFilePath = qsave_path + "/" + fileName + ".zip";

        QFile  selectFile(qfile_path);
        qint64 size = selectFile.size() / 1024 / 1024;
        if (!selectFile.open(QIODevice::ReadOnly) || size > FILE_MAX_SIZE) {
            // 打开文件失败，或者大于1GB导致无法压缩的文件
            return false;
        }
        QString    addFileName = QFileInfo(qfile_path).fileName();
        QZipWriter writer(writerFilePath);
        writer.addFile(addFileName, selectFile.readAll());
        selectFile.close();

        // 删除原文件
        if (remove) return delete_file_or_folder(file_path);

        return true;
    } else {
        // 压缩的是一个文件夹
        QString zipRootFolder  = qfile_path.mid(qfile_path.lastIndexOf("/") + 1);
        QString selectDirUpDir = qfile_path.left(qfile_path.lastIndexOf("/"));
        QString saveFilePath   = qsave_path + "/" + zipRootFolder + ".zip";

        QZipWriter writer(saveFilePath);
        writer.addDirectory(zipRootFolder);
        QFileInfoList fileList = ergodic_compression_file(&writer, selectDirUpDir, qfile_path);
        writer.close();

        // 删除原文件
        if (remove) return delete_file_or_folder(file_path);

        if (0 == fileList.size()) return true;
        return false;
    }
}

bool Utils::decompressionZipFile(const std::string &file_path, const std::string &save_path, bool remove) {
    // 创建解压缩后文件夹
    zel::fs::Directory save(save_path);
    save.create();

    QString qfile_path = QString(file_path.c_str());
    QString qsave_path = QString(save_path.c_str());

    if (qfile_path.isEmpty() || qsave_path.isEmpty()) {
        return false;
    }
    if (!QFileInfo(qfile_path).isFile() || !QFileInfo(qsave_path).isDir()) {
        return false;
    }

    bool                          ret = true;
    QZipReader                    zipReader(qfile_path);
    QVector<QZipReader::FileInfo> zipAllFiles = zipReader.fileInfoList();

    for (const QZipReader::FileInfo &zipFileInfo : zipAllFiles) {

        QString fullPath  = QDir(qsave_path).filePath(zipFileInfo.filePath);
        QString cleanPath = QDir::cleanPath(fullPath);

        if (!cleanPath.startsWith(QDir(qsave_path).absolutePath())) {
            ret = false;
            continue;
        }

        QFileInfo fileInfo(cleanPath);

        if (zipFileInfo.isDir) {
            QDir().mkpath(cleanPath);
            continue;
        }

        if (zipFileInfo.isFile) {
            QByteArray data = zipReader.fileData(zipFileInfo.filePath);
            QDir().mkpath(fileInfo.absolutePath());
            QFile file(cleanPath);
            if (!file.open(QIODevice::WriteOnly)) {
                ret = false;
                continue;
            }

            file.write(data);
            file.close();
        }
    }
    zipReader.close();

    if (remove) delete_file_or_folder(file_path);
    return ret;
}

void Utils::replaceStringInXlsx(const std::string &filename, const std::string &old_str, const std::string &new_str) {
    // 打开 Excel 文件
    xlnt::workbook workbook;
    workbook.load(filename);

    // 获取活动工作表
    xlnt::worksheet worksheet = workbook.active_sheet();

    // 遍历工作表中的所有单元格
    for (auto row : worksheet.rows()) {
        for (auto cell : row) {
            if (!cell.has_value()) continue;

            // 检查单元格是否包含字符串
            std::string cell_value = cell.to_string();

            // 替换字符串
            size_t pos = 0;
            while ((pos = cell_value.find(old_str, pos)) != std::string::npos) {
                cell_value.replace(pos, old_str.length(), new_str);
                pos += new_str.length();
            }
            // 更新单元格内容
            worksheet.cell(cell.reference()).value(cell_value);
        }
    }

    // 保存 Excel 文件
    workbook.save(filename);
}

bool Utils::ftpUploadDir(const std::string &local_dir, std::string &remote_path, const std::string &userpwd) {
    // 判断local_path是否是目录
    if (zel::fs::isDir(local_dir)) {
        // 如果是目录，执行目录上传逻辑
        auto walkFunc = [=](const zel::fs::Entry &entry) -> bool {
            if (!entry.isDir()) {
                auto file           = entry.file();
                auto local_file_gbk = zel::utility::String::utf8ToGbk(file.path());
                auto remote_file    = zel::fs::join(remote_path, file.name());
                if (!ftpUploadFile(local_file_gbk, remote_file, userpwd)) {
                    return false;
                }
            }
            return true;
        };
        zel::fs::walk(local_dir, walkFunc, true);
        return true;
    } else {
        // 如果是文件，执行文件上传逻辑
        auto local_file_gbk = zel::utility::String::utf8ToGbk(local_dir);
        if (!ftpUploadFile(local_file_gbk, remote_path, userpwd)) {
            return false;
        }
        return true;
    }
}

bool Utils::ftpUploadFile(const std::string &local_file, const std::string &ftp_url, const std::string &userpwd) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        log_error("Failed to init curl");
        return false;
    }

    FILE *file = fopen(local_file.c_str(), "rb");
    if (!file) {
        log_error("Failed to open local file: %s", local_file.c_str());
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, ftp_url.c_str()); // ftp://host/path/file.txt
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str()); // user:password
    curl_easy_setopt(curl, CURLOPT_READDATA, file);
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_RETRY);

    // 允许自动创建目录
    curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_RETRY);

    CURLcode res = curl_easy_perform(curl);
    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        log_error("FTP upload failed: %s", curl_easy_strerror(res));
        return false;
    }

    log_info("FTP upload successful: %s", ftp_url.c_str());
    return true;
}

QFileInfoList Utils::ergodic_compression_file(QZipWriter *writer, const QString &rootPath, QString dirPath) {
    QDir crrDir(dirPath);
    /// 解压失败的文件
    QFileInfoList errFileList;

    /// 添加文件
    QFileInfoList fileList = crrDir.entryInfoList(QDir::Files | QDir::Hidden);
    for (const QFileInfo &fileInfo : fileList) {
        QString subFilePath       = fileInfo.absoluteFilePath();
        QString zipWithinfilePath = subFilePath.mid(rootPath.size() + 1);

        QFile  file(subFilePath);
        qint64 size = file.size() / 1024 / 1024;
        if (!file.open(QIODevice::ReadOnly) || size > FILE_MAX_SIZE) {
            /// 打开文件失败，或者大于1GB导致无法解压的文件
            errFileList.append(fileInfo);
            continue;
        }
        writer->addFile(zipWithinfilePath, file.readAll());
        file.close();
    }

    /// 添加文件夹
    QFileInfoList folderList = crrDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &folderInfo : folderList) {
        QString subDirPath       = folderInfo.absoluteFilePath();
        QString zipWithinDirPath = subDirPath.mid(rootPath.size() + 1);

        writer->addDirectory(zipWithinDirPath);
        QFileInfoList child_file_list = ergodic_compression_file(writer, rootPath, subDirPath);
        errFileList.append(child_file_list);
    }

    return errFileList;
}

bool Utils::delete_file_or_folder(const std::string &str_path) {
    QString strPath = QString(str_path.c_str());
    if (strPath.isEmpty() || !QDir().exists(strPath)) // 是否传入了空的路径||路径是否存在
        return false;

    QFileInfo FileInfo(strPath);

    if (FileInfo.isFile()) // 如果是文件
        QFile::remove(strPath);
    else if (FileInfo.isDir()) // 如果是文件夹
    {
        QDir qDir(strPath);
        qDir.removeRecursively();
    }
    return true;
}