#include "public.h"
#include "QtGui/private/qzipreader_p.h"

#include <qdir>
#include <qfileinfo>

QString splitFormt(const QString &str, const QString &sep, int start, int end) {

    QStringList str_list = str.split(sep);
    QString     result   = "";

    if (end == -1) end = str_list.size();

    if (start < 0 || start > str_list.size() || end < 0 || end > str_list.size() || start > end) {
        return result;
    }

    for (int i = start - 1; i <= end - 1; i++) {
        if (i == end - 1) {
            result += str_list[i];
        } else {
            result += str_list[i] + sep;
        }
    }

    return result;
}

QString createFolder(QString folder_path) {
    QDir dir(folder_path);
    if (dir.exists(folder_path)) {
        return folder_path;
    }

    QString parentDir = createFolder(folder_path.mid(0, folder_path.lastIndexOf('/')));
    QString dirName   = folder_path.mid(folder_path.lastIndexOf('/') + 1);

    QDir parentPath(parentDir);
    if (!dirName.isEmpty()) {
        parentPath.mkpath(dirName);
    }
    return parentDir + "/" + dirName;
}

bool copyFolder(const QString &fromDir, const QString &toDir, bool coverFileIfExist) {
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);

    if (!targetDir.exists()) { // 如果目标目录不存在，则进行创建
        if (!targetDir.mkdir(targetDir.absolutePath())) return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    for (QFileInfo fileInfo : fileInfoList) {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") continue;
        // 当为目录时，递归的进行copy
        if (fileInfo.isDir()) {
            if (!copyFolder(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()), coverFileIfExist))
                return false;
        } else {
            // 当允许覆盖操作时，将旧文件进行删除操作
            if (coverFileIfExist && targetDir.exists(fileInfo.fileName())) {
                targetDir.remove(fileInfo.fileName());
            }

            // 进行文件拷贝
            if (!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))) {
                return false;
            }
        }
    }
    return true;
}

void renameFolder(const QString oldPath, const QString newPath) {
    // 重命名文件夹
    QDir dirOld(oldPath);
    dirOld.rename(oldPath, newPath);
}

bool deleteFileOrFolder(const QString &strPath) {
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

bool copyFile(QString sourceDir, QString toDir, bool coverFileIfExist) {
    toDir.replace("\\", "/");
    if (sourceDir == toDir) {
        return true;
    }
    if (!QFile::exists(sourceDir)) {
        return false;
    }
    QDir *createfile = new QDir;
    bool  exist      = createfile->exists(toDir);
    if (exist) {
        if (coverFileIfExist) {
            createfile->remove(toDir);
        }
    } // end if

    if (!QFile::copy(sourceDir, toDir)) {
        return false;
    }
    return true;
}

QFileInfoList ergodicCompressionFile(QZipWriter *writer, const QString &rootPath, QString dirPath) {
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
        QFileInfoList child_file_list = ergodicCompressionFile(writer, rootPath, subDirPath);
        errFileList.append(child_file_list);
    }

    return errFileList;
}

bool compressionZipFile(const QString &selectFile2DirPath, const QString &savePath) {
    if (selectFile2DirPath.isEmpty() || savePath.isEmpty()) {
        return false;
    }
    if (!QFile::exists(selectFile2DirPath) || !QFileInfo(savePath).isDir()) {
        return false;
    }

    // 压缩的是一个文件
    if (QFileInfo(selectFile2DirPath).isFile()) {
        QString fileName       = QFileInfo(selectFile2DirPath).baseName();
        QString writerFilePath = savePath + "/" + fileName + ".zip";

        QFile  selectFile(selectFile2DirPath);
        qint64 size = selectFile.size() / 1024 / 1024;
        if (!selectFile.open(QIODevice::ReadOnly) || size > FILE_MAX_SIZE) {
            // 打开文件失败，或者大于1GB导致无法压缩的文件
            return false;
        }
        QString    addFileName = QFileInfo(selectFile2DirPath).fileName();
        QZipWriter writer(writerFilePath);
        writer.addFile(addFileName, selectFile.readAll());
        selectFile.close();
        return true;
    } else {
        // 压缩的是一个文件夹
        QString zipRootFolder  = selectFile2DirPath.mid(selectFile2DirPath.lastIndexOf("/") + 1);
        QString selectDirUpDir = selectFile2DirPath.left(selectFile2DirPath.lastIndexOf("/"));
        QString saveFilePath   = savePath + "/" + zipRootFolder + ".zip";

        QZipWriter writer(saveFilePath);
        writer.addDirectory(zipRootFolder);
        QFileInfoList fileList = ergodicCompressionFile(&writer, selectDirUpDir, selectFile2DirPath);
        writer.close();
        if (0 == fileList.size()) return true;
        return false;
    }
}

bool compressionZipFile(const QString &file_path) {
    QString save_path = file_path.mid(0, file_path.lastIndexOf("/"));
    return compressionZipFile(file_path, save_path);
}

bool decompressionZipFile(const QString &selectZipFilePath, const QString &savePath) {
    if (selectZipFilePath.isEmpty() || savePath.isEmpty()) {
        return false;
    }
    if (!QFileInfo(selectZipFilePath).isFile() || !QFileInfo(savePath).isDir()) {
        return false;
    }

    bool                          ret = true;
    QZipReader                    zipReader(selectZipFilePath);
    QVector<QZipReader::FileInfo> zipAllFiles = zipReader.fileInfoList();
    for (const QZipReader::FileInfo &zipFileInfo : zipAllFiles) {
        const QString currDir2File = savePath + "/" + zipFileInfo.filePath;
        if (zipFileInfo.isSymLink) {
            QString destination = QFile::decodeName(zipReader.fileData(zipFileInfo.filePath));
            if (destination.isEmpty()) {
                ret = false;
                continue;
            }

            QFileInfo linkFi(currDir2File);
            if (!QFile::exists(linkFi.absolutePath())) QDir::root().mkpath(linkFi.absolutePath());
            if (!QFile::link(destination, currDir2File)) {
                ret = false;
                continue;
            }
        }
        if (zipFileInfo.isDir) {
            QDir(savePath).mkpath(currDir2File);
        }
        if (zipFileInfo.isFile) {
            QByteArray dt     = zipFileInfo.filePath.toUtf8();
            QString    strtmp = QString::fromLocal8Bit(dt);

            QFile currFile(currDir2File);
            if (!currFile.isOpen()) {
                currFile.open(QIODevice::WriteOnly);
            } else {
                ret = false;
                continue;
            }

            qint64 size = zipFileInfo.size / 1024 / 1024;
            if (size > FILE_MAX_SIZE) {
                ret = false;
                continue;
            }
            QByteArray byteArr = zipReader.fileData(strtmp);
            currFile.write(byteArr);
            currFile.setPermissions(zipFileInfo.permissions);
            currFile.close();
        }
    }
    zipReader.close();
    return ret;
}
