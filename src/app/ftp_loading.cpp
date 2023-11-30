#include "ftp_loading.h"

#include <QDebug>
#include <QMessageBox>
#include <QMovie>
#include <QThread>

FtpLoading::FtpLoading(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_FtpLoading) {
    ui_->setupUi(this);

    initWindow();

    initUI();

    initSignalSlot();
}

FtpLoading::~FtpLoading() { delete ui_; }

void FtpLoading::initWindow() { qRegisterMetaType<UploadPrd::Type>("UploadPrd::Type"); }

void FtpLoading::initUI() {

    // 新增图片
    QPixmap pixmap(":/image/waiting.png");
    // 设置图片大小
    ui_->ftp_gif_label->setPixmap(pixmap);
}

void FtpLoading::initSignalSlot() {}

void FtpLoading::startUpload(const QString &duration) {
    // 加载动画
    QMovie *movie = new QMovie(":/image/loading.gif");
    ui_->ftp_gif_label->setMovie(movie);

    movie->setScaledSize(QSize(32, 32));
    movie->start();
}

void FtpLoading::finish(const QString &duration) {

    // 插入图片
    QPixmap pixmap(":/image/success.png");
    ui_->ftp_gif_label->setPixmap(pixmap);

    ui_->ftp_text_label->setText(duration);
}

void FtpLoading::failure(UploadPrd::Type type, const QString &err_msg) {}
