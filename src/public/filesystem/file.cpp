#include "file.h"
#include "../utility/string.h"
#include "directory.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

namespace zel {
namespace filesystem {

File::File(const std::string &path) {
    path_  = absolutePath(path);
    wpath_ = utility::String::string2wstring(path_);
}

File::~File() {}

bool File::create() {
    if (exists()) {
        return true;
    }

    if (dir().empty()) {
        return false;
    }

    Directory d(dir());
    if (!d.exists()) {
        d.create();
        printf("1\n");
    }

    std::ofstream out(wpath_);
    if (!out) {
        return false;
    }
    out.close();
    return true;
}

void File::remove() {
    if (!exists()) {
        return;
    }

    DeleteFileW(wpath_.c_str());
}

bool File::copy(const std::string &dest) const {

    if (!exists()) {
        return false;
    }

    File file(dest);
    if (!file.exists()) {
        file.create();
    }

    if (CopyFileW(wpath_.c_str(), utility::String::string2wstring(dest).c_str(), false) == 0) {
        return false;
    }

    return true;
}

bool File::rename(const std::string &dest_filename) {
    std::string dest = dir() + "/" + dest_filename;
    return move(dest);
}

bool File::move(const std::string &dest) {
    if (!exists()) {
        return false;
    }
    if (dest == path_) {
        return true;
    }
    if (copy(dest)) {
        remove();
        return true;
    }

    return false;
}

bool File::exists() const {
    DWORD dwAttrib = GetFileAttributesW(wpath_.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void File::clear() {
    if (!exists()) {
        return;
    }
    std::ofstream out(wpath_);
    out.close();
}

std::string File::read() const {
    if (!exists()) {
        return "";
    }
    std::ifstream in(wpath_);
    if (!in) {
        return "";
    }
    std::string content;
    in.seekg(0, std::ios::end);
    content.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&content[0], content.size());
    in.close();
    return content;
}

bool File::write(const std::string &content) {
    if (!exists()) {
        return false;
    }
    std::ofstream out(wpath_);
    if (!out) {
        return false;
    }
    out << content;
    out.close();

    return true;
}

std::string File::name() const {
    std::string::size_type pos = path_.find_last_of('/');
    if (pos == std::string::npos) {
        return path_;
    }
    return path_.substr(pos + 1);
}

std::string File::path() const { return path_; }

std::string File::dir() const {
    std::string::size_type pos = path_.find_last_of('/');
    if (pos == std::string::npos) {
        return "";
    }
    return path_.substr(0, pos);
}

std::string File::time() const {
    if (!exists()) {
        return "";
    }
    struct stat buf;
    stat(path_.c_str(), &buf);
    return ctime(&buf.st_mtime);
}

int File::line() const {
    if (!exists()) {
        return 0;
    }
    std::ifstream in(wpath_);
    if (!in) {
        return 0;
    }
    int         count = 0;
    std::string line;
    while (getline(in, line)) {
        count++;
    }
    return count;
}

int File::size() const {
    if (!exists()) {
        return 0;
    }
    struct stat buf;
    stat(path_.c_str(), &buf);
    return buf.st_size;
}

std::string File::absolutePath(const std::string &path) {
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