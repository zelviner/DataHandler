#pragma once
#include "QtGui/private/qzipwriter_p.h"
#include <qfileinfolist>
#include <qstring>
#include <qstringlist>

#define FILE_MAX_SIZE 1024

class Utils {

  public:
    Utils();

    ~Utils();

    /// @brief 压缩 zip 文件到当前路径
    /// @param file_path
    /// @param save_path
    /// @param remove 是否需要删除原文件
    static bool compressionZipFile(const std::string &file_path, bool remove = false);

    /// @brief 压缩 zip 文件
    /// @param file_path
    /// @param save_path
    /// @param remove 是否需要删除原文件
    static bool compressionZipFile(const std::string &file_path, const std::string &save_path, bool remove = false);

    /// @brief 解压缩 zip 文件
    /// @param selectZipFilePath
    /// @param save_path
    /// @param remove 是否需要删除原文件
    static bool decompressionZipFile(const std::string &file_path, const std::string &save_path, bool remove = false);

    /// @brief 替换 xlsx 文件中的字符串
    /// @param filename xlsx 文件路径
    /// @param old_str 旧字符串
    /// @param new_str 新字符串
    static void replaceStringInXlsx(const std::string &filename, const std::string &old_str, const std::string &new_str);

  private:
    static QFileInfoList ergodicCompressionFile(QZipWriter *writer, const QString &rootPath, QString dirPath);

    /// @brief 删除文件或文件夹
    /// @param strPath 要删除的文件夹或文件的路径
    static bool deleteFileOrFolder(const std::string &str_path);
};