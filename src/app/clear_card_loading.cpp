#include "clear_card_loading.h"

#include <QDebug>
#include <QMessageBox>
#include <QMovie>
#include <QThread>

ClearCardLoading::ClearCardLoading(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_ClearCardLoading) {
    ui_->setupUi(this);

    initWindow();

    initUI();

    initSignalSlot();
}

ClearCardLoading::~ClearCardLoading() { delete ui_; }

void ClearCardLoading::initWindow() { qRegisterMetaType<ClearCard::Type>("ClearCard::Type"); }

void ClearCardLoading::initUI() {

    // 新增图片
    QPixmap pixmap(":/image/waiting.png");
    // 设置图片大小
    ui_->clear_label->setPixmap(pixmap);
}

void ClearCardLoading::initSignalSlot() {}

void ClearCardLoading::startClear(const QString &duration) {
    // 加载动画
    QMovie *movie = new QMovie(":/image/loading.gif");
    ui_->clear_label->setMovie(movie);

    movie->setScaledSize(QSize(32, 32));
    movie->start();
}

void ClearCardLoading::finish(const QString &duration) {

    // 插入图片
    QPixmap pixmap(":/image/success.png");
    ui_->clear_label->setPixmap(pixmap);

    ui_->clear_duration_label->setText(duration);
}

void ClearCardLoading::failure(ClearCard::Type type, const QString &err_msg) {

    switch (type) {

    case ClearCard::CONNECT: {
        hide();
        QMessageBox::critical(this, "错误", err_msg);
        break;
    }

    case ClearCard::CLEAR: {
        ui_->clear_label->setPixmap(QPixmap(":/image/failure.png"));
        ui_->clear_duration_label->setText(err_msg);
        break;
    }

    default:
        break;
    }
}

void ClearCardLoading::success(ClearCard::Type type, const QString &duration) {

    switch (type) {

    case ClearCard::CLEAR: {
        startClear(duration);
        break;
    }

    case ClearCard::FINISH: {
        finish(duration);
        break;
    }

    default:
        break;
    }
}