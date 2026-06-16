/// @file main.cpp
/// @author ZEL (zel1362848545@gmail.com)
/// @brief
/// @version 0.1
/// @date 2023-04-18
/// @copyright Copyright (c) 2023 ZEL

#include "app/main_window.h"

#include <qfile>
#include <qapplication>
#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[]) {

    // 设置高DPI
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);
    a.setStyle("Fusion");
    MainWindow   w;

    // 设置全局样式
    QFile file(":/qss/windows.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        a.setStyleSheet(QString::fromUtf8(file.readAll()));
        file.close();
    }

    w.show();
    return a.exec();
}
