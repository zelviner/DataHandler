#include "loading.h"
#include "task/card.h"
#include "task/clear_card.hpp"
#include "task/input_card.hpp"

#include <QDebug>
#include <QMovie>
#include <QThread>

Loading::Loading(ScriptInfo *script_info, const std::string &personal_data, QMainWindow *parent)
    : QMainWindow(parent)
    , ui(new Ui_Loading)
    , script_info_(script_info)
    , personal_data_(personal_data) {
    ui->setupUi(this);

    initWindow();

    initUI();

    initSignalSlot();
}

Loading::~Loading() { delete ui; }

bool Loading::inputCard() {
    auto card = std::make_shared<Card>(script_info_, personal_data_);

    // 创建工作线程
    auto input_card = new InputCard(card);

    // 连接信号槽
    connect(input_card, &InputCard::resultReady, this, &Loading::resultReady);
    connect(input_card, &InputCard::success, this, &Loading::success);

    // 启动工作线程
    input_card->start();

    return true;
}

void Loading::initWindow() {}

void Loading::initUI() {

    // 加载gif动画
    QMovie *prepersonalMovie  = new QMovie(":/image/loading.gif");
    QMovie *postpersonalMovie = new QMovie(":/image/loading.gif");
    QMovie *checkMovie        = new QMovie(":/image/loading.gif");

    // 设置动画对象到对应的QLabel
    ui->prepersonal_label->setMovie(prepersonalMovie);
    ui->postpersonal_label->setMovie(postpersonalMovie);
    ui->check_label->setMovie(checkMovie);

    // 设置动画大小
    prepersonalMovie->setScaledSize(QSize(48, 48));
    postpersonalMovie->setScaledSize(QSize(48, 48));
    checkMovie->setScaledSize(QSize(48, 48));

    ui->prepersonal_label->setFixedSize(48, 48);
    ui->postpersonal_label->setFixedSize(48, 48);
    ui->check_label->setFixedSize(48, 48);

    // // 设置动画速度
    // movie->setSpeed(100);

    // // 开始播放动画
    // movie->start();

    ui->prepersonal_label->movie()->start();


    // 新增图片
    QPixmap pixmap(":/image/not_started.png");
    // 设置图片大小
    pixmap = pixmap.scaled(48, 48);
    ui->postpersonal_label->setPixmap(pixmap);
    ui->check_label->setPixmap(pixmap);
}

void Loading::initSignalSlot() {}

void Loading::resultReady(const QString &s) { qDebug() << s; }

void Loading::success(const int a) {
    switch (a) {
    case 1: {
        // ui->prepersonal_label->movie()->stop();
        QPixmap pixmap(":/image/success.png");
        pixmap = pixmap.scaled(48, 48);
        ui->prepersonal_label->setPixmap(pixmap);
        ui->postpersonal_label->movie()->start();
        break;
    }
    case 2: {
        // ui->postpersonal_label->movie()->stop();
        QPixmap pixmap(":/image/success.png");
        pixmap = pixmap.scaled(48, 48);
        ui->postpersonal_label->setPixmap(pixmap);
        ui->check_label->movie()->start();
        break;
    }
    case 3: {
        // ui->check_label->movie()->stop();
        QPixmap pixmap(":/image/success.png");
        pixmap = pixmap.scaled(48, 48);
        ui->check_label->setPixmap(pixmap);
        break;
    }
    default:
        break;
    }
}