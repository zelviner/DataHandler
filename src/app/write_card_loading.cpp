#include "write_card_loading.h"

#include <qdebug>
#include <qmessagebox>
#include <qmovie>
#include <qthread>

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
    QString str = atr;
    emit    bareAtr(str);
}

void WriteCardLoading::prePersonal(const QString &duration, const QString &apdu_response) { ui_->prepersonal_duration_label->setText(apdu_response); }

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
    QString str = atr;
    emit    whiteAtr(str);
}

void WriteCardLoading::postPersonal(const QString &duration, const QString &apdu_response) { ui_->postpersonal_duration_label->setText(apdu_response); }

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

void WriteCardLoading::check(const QString &duration, const QString &apdu_response) { ui_->check_duration_label->setText(apdu_response); }

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

void WriteCardLoading::success(WriteCard::Type type, const QString &duration, const QString &apdu_response) {

    switch (type) {

    case WriteCard::BARE_ATR: {
        startPrePersonal(duration, apdu_response);
        break;
    }

    case WriteCard::PREPERSONAL: {
        prePersonal(duration, apdu_response);
        break;
    }

    case WriteCard::WHITE_ATR: {
        startPostPersonal(duration, apdu_response);
        break;
    }

    case WriteCard::POSTPERSONAL: {
        postPersonal(duration, apdu_response);
        break;
    }

    case WriteCard::FINISHED_ATR: {
        startCheck(duration, apdu_response);
        break;
    }

    case WriteCard::CHECK: {
        check(duration, apdu_response);
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