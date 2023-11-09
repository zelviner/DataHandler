#include "path.h"

Path::Path(QString dir_path)
    : dir_path_(dir_path) {}

Path::~Path() {}

void Path::show() {
    qDebug() << "dir_path: " << dir_path_;

    qDebug() << "order_path: " << path_.order_path;
    qDebug() << "ftp_data_path: " << path_.ftp_data_path;
    qDebug() << "script_path: " << path_.script_path;
    qDebug() << "src_tag_data_path: " << path_.src_tag_data_path;

    qDebug() << "data_path: " << path_.data_path;

    qDebug() << "temp_path: " << path_.temp_path;
    qDebug() << "screenshot_path: " << path_.screenshot_path;
    qDebug() << "print_path: " << path_.print_path;
    qDebug() << "dest_tag_data_path: " << path_.dest_tag_data_path;

    qDebug() << "authentication_path: " << path_.authentication_path;

    qDebug() << "clear_card_path: " << path_.clear_card_path;
}

QString Path::dirPath() { return dir_path_; }

QString Path::orderPath() { return abslutePath(path_.order_path); }

QString Path::dataPath() { return abslutePath(path_.data_path); }

QString Path::ftpDataPath() { return abslutePath(path_.ftp_data_path); }

QString Path::scriptPath() { return abslutePath(path_.script_path); }

QString Path::screenshotPath() { return abslutePath(path_.screenshot_path); }

QString Path::authenticationPath() { return abslutePath(path_.authentication_path); }

QString Path::tempPath() { return abslutePath(path_.temp_path); }

QString Path::srcTagDataPath() { return abslutePath(path_.src_tag_data_path); }

QString Path::destTagDataPath() { return abslutePath(path_.dest_tag_data_path); }

QString Path::clearCardPath() { return abslutePath(path_.clear_card_path); }

QString Path::printPath() { return abslutePath(path_.print_path); }

QStringList Path::printsPath() {
    QStringList paths;
    for (auto path : path_.print_paths) {
        paths << abslutePath(path);
    }
    return paths;
}

void Path::dirPath(QString path) { dir_path_ = path; }

void Path::orderPath(QString path) { path_.order_path = path; }

void Path::dataPath(QString path) { path_.data_path = path; }

void Path::ftpDataPath(QString path) { path_.ftp_data_path = path; }

void Path::scriptPath(QString path) { path_.script_path = path; }

void Path::screenshotPath(QString path) { path_.screenshot_path = path; }

void Path::authenticationPath(QString path) { path_.authentication_path = path; }

void Path::tempPath(QString path) { path_.temp_path = path; }

void Path::srcTagDataPath(QString path) { path_.src_tag_data_path = path; }

void Path::destTagDataPath(QString path) { path_.dest_tag_data_path = path; }

void Path::clearCardPath(QString path) { path_.clear_card_path = path; }

void Path::printPath(QString path) { path_.print_path = path; }

void Path::printsPath(QStringList path_list) { path_.print_paths = path_list; }

QString Path::abslutePath(QString path) {
    if (path.indexOf(":") != -1) return path;
    return dir_path_ + "/" + path;
}