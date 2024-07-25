#include "path.h"
#include <vector>

Path::Path(std::string datagram_path)
    : datagram_path_(datagram_path) {}

Path::~Path() {}

void Path::show() {
    printf("datagram_path: %s\n", datagram_path_.c_str());
    printf("dir_path: %s\n", dir_path_.c_str());
    printf("data_path: %s\n", path_.data_path.c_str());
    printf("temp_path: %s\n", path_.temp_path.c_str());
    printf("screenshot_path: %s\n", path_.screenshot_path.c_str());
    printf("print_path: %s\n", path_.print_path.c_str());
    printf("tag_data_path: %s\n", path_.tag_data_path.c_str());
    printf("script_path: %s\n", path_.script_path.c_str());
}

std::string Path::datagramPath() { return datagram_path_; }

std::string Path::dirPath() { return dir_path_; }

std::string Path::dataPath() { return path_.data_path; }

std::string Path::scriptPath() { return path_.script_path; }

std::string Path::screenshotPath() { return path_.screenshot_path; }

std::string Path::tempPath() { return path_.temp_path; }

std::string Path::tagDataPath() { return path_.tag_data_path; }

std::string Path::printPath() { return path_.print_path; }

std::vector<std::string> Path::printPaths() {
    std::vector<std::string> paths;
    for (auto path : path_.print_paths) {
        paths.push_back(abslutePath(path));
    }
    return paths;
}

void Path::dirPath(std::string path) { dir_path_ = path; }

void Path::dataPath(std::string path) { path_.data_path = path; }

void Path::scriptPath(std::string path) { path_.script_path = path; }

void Path::screenshotPath(std::string path) { path_.screenshot_path = path; }

void Path::tempPath(std::string path) { path_.temp_path = path; }

void Path::tagDataPath(std::string path) { path_.tag_data_path = path; }

void Path::printPath(std::string path) { path_.print_path = path; }

void Path::printPaths(std::vector<std::string> path_list) { path_.print_paths = path_list; }

std::string Path::abslutePath(std::string path) {
    if (path.find_first_of(":") == std::string::npos) return path;
    return dir_path_ + "/" + path;
}