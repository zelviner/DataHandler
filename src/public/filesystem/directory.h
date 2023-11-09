#pragma once

#include <string>
#include <vector>

#include "file.h"

namespace zel {
namespace filesystem {

class Directory {

  public:
    Directory(const std::string &path);
    ~Directory();

    /// @brief 创建一个空目录(含子目录)
    void create();

    /// @brief 删除目录(含子目录)
    bool remove();

    /// @brief 切换目录
    /// @param path 目标目录路径
    bool cd(const std::string &path);

    /// @brief 复制目录(含子目录)
    /// @param dest 目标目录路径
    bool copy(const std::string &dest) const;

    /// @brief 重命名目录(含子目录)
    /// @param dest 目标目录路径
    bool rename(const std::string &dest_dirname);

    /// @brief 移动目录(含子目录)
    /// @param dest 目标目录路径
    bool move(const std::string &dest);

    /// @brief 判断目录是否存在
    bool exists() const;

    /// @brief 清空目录(含子目录)
    void clear();

    /// @brief 获取目录的绝对路径
    std::string path() const;

    /// @brief 获取目录(包含子目录)下全部文件
    /// @param recursive 是否递归
    /// @param full_path 是否返回全路径
    std::vector<File> files(bool recursive = false, bool full_path = false) const;

    /// @brief 获取目录(包含子目录)下全部文件夹
    /// @param recursive 是否递归
    /// @param full_path 是否返回全路径
    std::vector<Directory> dirs(bool recursive = false, bool full_path = false) const;

    /// @brief 获取目录(包含子目录)下包含多少个文件
    /// @param recursive 是否递归
    int count(bool recursive = false) const;

    /// @brief 获取目录(包含子目录)下全部文件的行数
    /// @param recursive 是否递归
    int line(bool recursive = false) const;

    /// @brief 获取目录(包含子目录)下全部文件的大小
    /// @param recursive 是否递归
    int size(bool recursive = false) const;

  private:
    /// @brief 相对路径转绝对路径
    /// @param path 相对路径
    std::string absolutePath(const std::string &path);

  private:
    std::string  path_;  // 目录路径
    std::wstring wpath_; // 目录路径
};

} // namespace filesystem
} // namespace zel