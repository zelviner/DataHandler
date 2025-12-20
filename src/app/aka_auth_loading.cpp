#include "aka_auth_loading.h"

#include <qdebug>
#include <qmessagebox>
#include <qmovie>
#include <qthread>

AkaAuthLoading::AkaAuthLoading(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_AkaAuthLoading) {
    ui_->setupUi(this);

    init_window();

    init_ui();

    init_signal_slot();
}

AkaAuthLoading::~AkaAuthLoading() { delete ui_; }

void AkaAuthLoading::init_window() { qRegisterMetaType<AkaAuth::Type>("AkaAuth::Type"); }

void AkaAuthLoading::init_ui() {

    // 新增图片
    QPixmap pixmap(":/image/waiting.png");
    // 设置图片大小
    ui_->aka_auth_label->setPixmap(pixmap);
}

void AkaAuthLoading::init_signal_slot() {}

void AkaAuthLoading::start_auth() {
    // 加载动画
    QMovie *movie = new QMovie(":/image/loading.gif");
    ui_->aka_auth_label->setMovie(movie);

    movie->setScaledSize(QSize(32, 32));
    movie->start();
}

void AkaAuthLoading::auth(const QString &apdu_response) { ui_->aka_auth_duration_label->setText(apdu_response); }

void AkaAuthLoading::finish(const QString &duration) {

    // 插入图片
    QPixmap pixmap(":/image/success.png");
    ui_->aka_auth_label->setPixmap(pixmap);

    ui_->aka_auth_duration_label->setText(duration);
}

void AkaAuthLoading::failure(AkaAuth::Type type, const QString &err_msg) {

    switch (type) {

    case AkaAuth::CONNECT: {
        hide();
        QMessageBox::critical(this, "错误", err_msg);
        break;
    }

    case AkaAuth::AUTH: {
        ui_->aka_auth_label->setPixmap(QPixmap(":/image/failure.png"));
        ui_->aka_auth_duration_label->setText(err_msg);
        break;
    }

    default:
        break;
    }
}

void AkaAuthLoading::success(AkaAuth::Type type, const QString &duration, const QString &apdu_response) {

    switch (type) {

    case AkaAuth::START: {
        start_auth();
        break;
    }

    case AkaAuth::AUTH: {
        auth(apdu_response);
        break;
    }

    case AkaAuth::FINISH: {
        finish(duration);
        break;
    }

    default:
        break;
    }
}