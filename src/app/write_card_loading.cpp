#include "write_card_loading.h"

#include <QDebug>
#include <QMessageBox>
#include <QMovie>
#include <QThread>

WriteCardLoading::WriteCardLoading(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_WriteCardLoading) {
    ui_->setupUi(this);

    initWindow();

    initUI();

    initSignalSlot();
}

WriteCardLoading::~WriteCardLoading() { delete ui_; }

void WriteCardLoading::initWindow() { qRegisterMetaType<WriteCard::Type>("WriteCard::Type"); }

void WriteCardLoading::initUI() {

    // 新增图片
    QPixmap pixmap(":/image/waiting.png");
    // 设置图片大小
    ui_->prepersonal_label->setPixmap(pixmap);
    ui_->postpersonal_label->setPixmap(pixmap);
    ui_->check_label->setPixmap(pixmap);
}

void WriteCardLoading::initSignalSlot() {}

void WriteCardLoading::startPrePersonal(const QString &duration, const QString &atr) {
    // 加载动画
    QMovie *movie = new QMovie(":/image/loading.gif");
    ui_->prepersonal_label->setMovie(movie);

    movie->setScaledSize(QSize(32, 32));
    movie->start();
    emit bareAtr(atr);
}

void WriteCardLoading::startPostPersonal(const QString &duration, const QString &atr) {

    // 插入图片
    QPixmap pixmap(":/image/success.png");
    ui_->prepersonal_label->setPixmap(pixmap);

    // 加载动画
    QMovie *movie = new QMovie(":/image/loading.gif");
    ui_->postpersonal_label->setMovie(movie);

    movie->setScaledSize(QSize(32, 32));
    movie->start();

    ui_->prepersonal_duration_label->setText(duration);
    emit whiteAtr(atr);
}

void WriteCardLoading::startCheck(const QString &duration, const QString &atr) {

    // 插入图片
    QPixmap pixmap(":/image/success.png");
    ui_->postpersonal_label->setPixmap(pixmap);

    // 加载动画
    QMovie *movie = new QMovie(":/image/loading.gif");
    ui_->check_label->setMovie(movie);

    movie->setScaledSize(QSize(32, 32));
    movie->start();

    ui_->postpersonal_duration_label->setText(duration);

    emit finishedAtr(atr);
}

void WriteCardLoading::finish(const QString &duration) {

    // 插入图片
    QPixmap pixmap(":/image/success.png");
    ui_->check_label->setPixmap(pixmap);

    ui_->check_duration_label->setText(duration);
}

void WriteCardLoading::failure(WriteCard::Type type, const QString &err_msg) {

    switch (type) {

    case WriteCard::CONNECT: {
        hide();
        QMessageBox::critical(this, "错误", err_msg);
        break;
    }

    case WriteCard::PREPERSONAL: {
        ui_->prepersonal_label->setPixmap(QPixmap(":/image/failure.png"));
        ui_->prepersonal_duration_label->setText(err_msg);
        break;
    }

    case WriteCard::POSTPERSONAL: {
        ui_->postpersonal_label->setPixmap(QPixmap(":/image/failure.png"));
        ui_->postpersonal_duration_label->setText(err_msg);
        break;
    }

    case WriteCard::CHECK: {
        ui_->check_label->setPixmap(QPixmap(":/image/failure.png"));
        ui_->check_duration_label->setText(err_msg);
        break;
    }

    default:
        break;
    }
}

void WriteCardLoading::success(WriteCard::Type type, const QString &duration, const QString &atr) {

    switch (type) {

    case WriteCard::PREPERSONAL: {
        startPrePersonal(duration, atr);
        break;
    }

    case WriteCard::POSTPERSONAL: {
        startPostPersonal(duration, atr);
        break;
    }

    case WriteCard::CHECK: {
        startCheck(duration, atr);
        break;
    }

    case WriteCard::FINISH: {
        finish(duration);
        break;
    }

    default:
        break;
    }
}