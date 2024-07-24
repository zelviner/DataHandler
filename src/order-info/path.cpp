#include "path.h"

Path::Path(QString datagram_path)
    : datagram_path_(datagram_path) {}

Path::~Path() {}

void Path::show() {
    qDebug() << "dir_path: " << dir_path_;
    qDebug() << "zh_order_path: " << path_.zh_order_path;
    qDebug() << "zh_data_path: " << path_.zh_data_path;
    qDebug() << "zh_script_path: " << path_.zh_script_path;
    qDebug() << "zh_tag_data_path: " << path_.zh_tag_data_path;
    qDebug() << "data_path: " << path_.data_path;
    qDebug() << "temp_path: " << path_.temp_path;
    qDebug() << "screenshot_path: " << path_.screenshot_path;
    qDebug() << "print_path: " << path_.print_path;
    qDebug() << "tag_data_path: " << path_.tag_data_path;
    qDebug() << "authentication_path: " << path_.authentication_path;
    qDebug() << "script_path: " << path_.script_path;
    qDebug() << "clear_card_path: " << path_.clear_card_path;
}

QString Path::datagramPath() { return datagram_path_; }

QString Path::dirPath() { return dir_path_; }

QString Path::zhOrderPath() { return abslutePath(path_.zh_order_path); }

QString Path::zhDataPath() { return abslutePath(path_.zh_data_path); }

QString Path::zhScriptPath() { return abslutePath(path_.zh_script_path); }

QString Path::zhTagDataPath() { return abslutePath(path_.zh_tag_data_path); }

QStringList Path::zhPrintPaths() {
    QStringList paths;
    for (auto path : path_.zh_print_paths) {
        paths << abslutePath(path);
    }
    return paths;
}

QString Path::dataPath() { return abslutePath(path_.data_path); }

QString Path::scriptPath() { return abslutePath(path_.script_path); }

QString Path::screenshotPath() { return abslutePath(path_.screenshot_path); }

QString Path::authenticationPath() { return abslutePath(path_.authentication_path); }

QString Path::tempPath() { return abslutePath(path_.temp_path); }

QString Path::tagDataPath() { return abslutePath(path_.tag_data_path); }

QString Path::clearCardPath() { return abslutePath(path_.clear_card_path); }

QString Path::printPath() { return abslutePath(path_.print_path); }

void Path::dirPath(QString path) { dir_path_ = path; }

void Path::zhOrderPath(QString path) { path_.zh_order_path = path; }

void Path::zhDataPath(QString path) { path_.zh_data_path = path; }

void Path::zhScriptPath(QString path) { path_.zh_script_path = path; }

void Path::zhTagDataPath(QString path) { path_.zh_tag_data_path = path; }

void Path::zhPrintPaths(QStringList path_list) { path_.zh_print_paths = path_list; }

void Path::dataPath(QString path) { path_.data_path = path; }

void Path::scriptPath(QString path) { path_.script_path = path; }

void Path::screenshotPath(QString path) { path_.screenshot_path = path; }

void Path::authenticationPath(QString path) { path_.authentication_path = path; }

void Path::tempPath(QString path) { path_.temp_path = path; }

void Path::tagDataPath(QString path) { path_.tag_data_path = path; }

void Path::clearCardPath(QString path) { path_.clear_card_path = path; }

void Path::printPath(QString path) { path_.print_path = path; }

QString Path::abslutePath(QString path) {
    if (path.indexOf(":") != -1) return path;
    return dir_path_ + "/" + path;
}