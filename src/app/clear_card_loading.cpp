#include "clear_card_loading.h"

#include <qdebug>
#include <qmessagebox>
#include <qmovie>
#include <qthread>
#include <chrono>

ClearCardLoading::ClearCardLoading(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_ClearCardLoading) {
    ui_->setupUi(this);

    init_window();

    init_ui();

    init_signal_slot();
}

ClearCardLoading::~ClearCardLoading() { delete ui_; }

void ClearCardLoading::init_window() { qRegisterMetaType<ClearCard::Type>("ClearCard::Type"); }

void ClearCardLoading::init_ui() {

    // 新增图片
    QPixmap pixmap(":/image/waiting.png");
    // 设置图片大小
    ui_->clear_label->setPixmap(pixmap);
}

void ClearCardLoading::init_signal_slot() {}

void ClearCardLoading::start_clear() {
    // 加载动画
    QMovie *movie = new QMovie(":/image/loading.gif");
    ui_->clear_label->setMovie(movie);

    movie->setScaledSize(QSize(32, 32));
    movie->start();
}

void ClearCardLoading::clear(const QString &apdu_response) { ui_->clear_duration_label->setText(apdu_response); }

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

void ClearCardLoading::success(ClearCard::Type type, const QString &duration, const QString &apdu_response) {

    switch (type) {

    case ClearCard::START: {
        start_clear();
        break;
    }

    case ClearCard::CLEAR: {
        clear(apdu_response);
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