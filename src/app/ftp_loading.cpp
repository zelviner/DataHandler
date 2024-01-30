#include "ftp_loading.h"

#include <qdebug>
#include <qdialog.h>
#include <qdialog>
#include <qwidget>
#include <qmessagebox>
#include <qthread>

FtpLoading::FtpLoading(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui_FtpLoading)
    , movie_(nullptr) {
    ui_->setupUi(this);

    // 设置透明度
    this->setWindowOpacity(0.8);

    // 取消对话框标题
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint); // 设置为对话框风格，并且去掉边框
    setWindowModality(Qt::WindowModal); // 设置为模式对话框，同时在构造该对话框时要设置父窗口
    ui_->ftp_gif_label->setStyleSheet("background-color: transparent;");
    movie_ = new QMovie(":/image/loading.gif");
    movie_->setScaledSize(QSize(32, 32));
    ui_->ftp_gif_label->setMovie(movie_);
    ui_->ftp_gif_label->setScaledContents(true);
    movie_->start();
}

FtpLoading::~FtpLoading() {
    delete ui_;
    movie_->stop();
}
