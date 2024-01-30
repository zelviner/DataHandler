#pragma once
#include "QtGui/private/qzipwriter_p.h"
#include <qfileinfolist>
#include <qstring>
#include <qstringlist>

#define FILE_MAX_SIZE 1024

/// @brief 截取待分隔符的字符串
/// @param str 带截取字符串
/// @param sep 分隔符: ",", " ", "_t
/// @param start 起始位置
/// @param end 结束位置
/// @return QString
QString splitFormt(const QString &str, const QString &sep, int start, int end = -1);

/// @brief 创建文件夹
/// @param folderPath 文件夹路径
QString createFolder(QString folder_path);

/// @brief 复制文件夹
/// @param fromDir 原路径
/// @param toDir 新路径
/// @param coverFileIfExist 如果存在是否覆盖
bool copyFolder(const QString &fromDir, const QString &toDir, bool coverFileIfExist);

/// @brief 重命名文件夹
/// @param oldPath 文件夹旧名称（全路径）
/// @param newPath 文件夹新名称（全路径）
void renameFolder(const QString oldPath, const QString newPath);

/// @brief 删除文件或文件夹
/// @param strPath 要删除的文件夹或文件的路径
bool deleteFileOrFolder(const QString &strPath);

/// @brief 拷贝文件
/// @param sourceDir 原文件路径
/// @param toDir 目标文件路径
/// @param coverFileIfExist  如果存在是否覆盖
bool copyFile(QString sourceDir, QString toDir, bool coverFileIfExist);

QFileInfoList ergodicCompressionFile(QZipWriter *writer, const QString &rootPath, QString dirPath);

/// @brief 压缩 zip 文件
/// @param selectFile2DirPath
/// @param savePath
bool compressionZipFile(const QString &selectFile2DirPath, const QString &savePath);

/// @brief 压缩 zip 文件到当前路径
/// @param selectFile2DirPath
/// @param savePath
bool compressionZipFile(const QString &selectFile2DirPath);

/// @brief 解压缩 zip 文件
/// @param selectZipFilePath
/// @param savePath
bool decompressionZipFile(const QString &selectZipFilePath, const QString &savePath);
