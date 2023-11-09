#include "directory.h"
#include "../utility/string.h"

#include <iostream>
#include <locale>
#include <string>
#include <windows.h>

namespace zel {
namespace filesystem {

Directory::Directory(const std::string &path) {
    path_  = absolutePath(path);
    wpath_ = utility::String::string2wstring(path_);
}

Directory::~Directory() {}

std::string Directory::path() const { return path_; }

void Directory::create() {
    if (exists()) {
        return;
    }

    std::string::size_type pos = path_.find_last_of('/');
    if (pos == std::string::npos) {
        return;
    }
    std::string dir = path_.substr(0, pos);

    if (dir.empty()) {
        return;
    }
    Directory d(dir);
    d.create();
    if (CreateDirectoryW(wpath_.c_str(), NULL) == 0) return;
}

bool Directory::remove() {
    if (!exists()) {
        return false;
    }
    std::vector<File> files = this->files(true);
    for (auto &file : files) {
        file.remove();
    }

    std::vector<Directory> dirs = this->dirs(true);
    for (auto &dir : dirs) {
        RemoveDirectoryW(dir.wpath_.c_str());
    }

    return true;
}

bool Directory::cd(const std::string &path) {
    if (!exists()) {
        return false;
    }

    std::string absolute_path = "";

    if (path.find("..") != std::string::npos) {
        std::string temp = path;
        while (temp.find("..") != std::string::npos) {
            std::string::size_type pos = path_.find_last_of('/');
            if (pos == std::string::npos) {
                return "";
            }

            path_ = path_.substr(0, pos);
            temp.replace(0, 3, "");
            absolute_path = path_ + temp;
        }
    } else if (path.find(".") != std::string::npos) {
        absolute_path = path_ + "/" + path.substr(2);
    } else {
        absolute_path = path_ + "/" + path;
    }

    Directory dir(absolute_path);
    if (!dir.exists()) {
        return false;
    }
    path_  = dir.path();
    wpath_ = utility::String::string2wstring(path_);
    return true;
}

bool Directory::copy(const std::string &dest) const {
    if (!exists()) {
        return false;
    }

    std::vector<File> files = this->files(true);
    for (auto &file : files) {
        std::string path = dest + "/" + file.dir().substr(path_.find_last_of('/') + 1) + "/" + file.name();
        if (!file.copy(path)) {
            return false;
        }
    }

    return true;
}

bool Directory::rename(const std::string &dest_dirname) {
    if (!exists()) {
        return false;
    }
    std::string dest_path = path_.find_first_of('/') == std::string::npos
        ? dest_dirname
        : path_.substr(0, path_.find_last_of('/')) + "/" + dest_dirname;

    return move(dest_path);
}

bool Directory::move(const std::string &dest) {
    if (!exists()) {
        return false;
    }
    Directory d(dest);
    d.create();
    std::vector<File> files = this->files(true);
    for (auto &file : files) {
        std::string            path = file.path();
        std::string            dest = path;
        std::string::size_type pos  = dest.find(path_);
        if (pos != std::string::npos) {
            dest.replace(pos, path_.size(), dest);
        }
        file.move(dest);
    }

    return true;
}

bool Directory::exists() const {
    DWORD dwAttrib = GetFileAttributesW(wpath_.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void Directory::clear() {
    if (!exists()) {
        return;
    }
    std::vector<File> files = this->files(true);
    for (auto &file : files) {
        file.clear();
    }
}

std::vector<File> Directory::files(bool recursive, bool full_path) const {

    std::vector<File> files;
    if (!exists()) {
        return files;
    }

    std::wstring path = wpath_;

    if (path[path.size() - 1] != '/') {
        path += '/';
    }

    std::wstring     search_path = path + L"*.*";
    WIN32_FIND_DATAW fd;
    HANDLE           hFind = ::FindFirstFileW(search_path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(fd.cFileName, L".") != 0 && wcscmp(fd.cFileName, L"..") != 0) {
                std::wstring file_path = path + fd.cFileName;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (recursive) {
                        Directory         d(utility::String::wstring2string(file_path));
                        std::vector<File> fs = d.files(true, full_path);
                        files.insert(files.end(), fs.begin(), fs.end());
                    }
                } else {
                    File file(utility::String::wstring2string(file_path));
                    files.push_back(file);
                }
            }
        } while (::FindNextFileW(hFind, &fd));
        ::FindClose(hFind);
    }
    return files;
}

std::vector<Directory> Directory::dirs(bool recursive, bool full_path) const {
    std::vector<Directory> dirs;
    if (!exists()) {
        return dirs;
    }

    std::wstring path = wpath_;
    if (path[path.size() - 1] != '/') {
        path += '/';
    }
    std::wstring     search_path = path + L"*.*";
    WIN32_FIND_DATAW fd;
    HANDLE           hFind = ::FindFirstFileW(search_path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(fd.cFileName, L".") != 0 && wcscmp(fd.cFileName, L"..") != 0) {
                std::wstring file_path = path + fd.cFileName;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (recursive) {
                        Directory              d(utility::String::wstring2string(file_path));
                        std::vector<Directory> ds = d.dirs(true, full_path);
                        dirs.insert(dirs.end(), ds.begin(), ds.end());
                    }

                    dirs.push_back(Directory(utility::String::wstring2string(file_path)));
                } else {
                    File file(utility::String::wstring2string(file_path));
                    if (full_path) {
                        dirs.push_back(Directory(file.dir()));
                    } else {
                        dirs.push_back(Directory(file.dir()));
                    }
                }
            }
        } while (::FindNextFileW(hFind, &fd));
        ::FindClose(hFind);
    }
    return dirs;
}

int Directory::count(bool recursive) const {
    if (!exists()) {
        return -1;
    }
    auto count = files(recursive).size();

    return count;
}

int Directory::line(bool recursive) const {
    int line = 0;
    if (!exists()) {
        return line;
    }
    // 使用wpath
    std::wstring path = wpath_;
    if (path[path.size() - 1] != '/') {
        path += '/';
    }
    std::wstring     search_path = path + L"*.*";
    WIN32_FIND_DATAW fd;
    HANDLE           hFind = ::FindFirstFileW(search_path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(fd.cFileName, L".") != 0 && wcscmp(fd.cFileName, L"..") != 0) {
                std::wstring file_path = path + fd.cFileName;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (recursive) {
                        Directory d(utility::String::wstring2string(file_path));
                        line += d.line(true);
                    }
                } else {
                    File file(utility::String::wstring2string(file_path));
                    line += file.line();
                }
            }
        } while (::FindNextFileW(hFind, &fd));
        ::FindClose(hFind);
    }
    return line;
}

int Directory::size(bool recursive) const {
    int size = 0;
    if (!exists()) {
        return size;
    }
    std::string path = path_;
    if (path[path.size() - 1] != '/') {
        path += '/';
    }
    std::string     search_path = path + "*.*";
    WIN32_FIND_DATA fd;
    HANDLE          hFind = ::FindFirstFile(search_path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                std::string file_path = path + fd.cFileName;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (recursive) {
                        Directory d(file_path);
                        size += d.size(true);
                    }
                } else {
                    File file(file_path);
                    size += file.size();
                }
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
    return size;
}

std::string Directory::absolutePath(const std::string &path) {
    if (path.find(':') != std::string::npos) {
        return path;
    }

    char buffer[MAX_PATH];
    GetFullPathName(path.c_str(), MAX_PATH, buffer, NULL);

    std::string absolute_path = std::string(buffer);

    // 把 \\ 替换成 /
    std::string::size_type pos = absolute_path.find("\\");
    while (pos != std::string::npos) {
        absolute_path.replace(pos, 1, "/");
        pos = absolute_path.find("\\");
    }

    return absolute_path;
}

} // namespace filesystem
} // namespace zel
