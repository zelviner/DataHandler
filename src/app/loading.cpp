#include "loading.h"
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

void Loading::initWindow() { qRegisterMetaType<Card::Type>("Card::Type"); }

void Loading::initUI() {

    // 新增图片
    QPixmap pixmap(":/image/waiting.png");
    // 设置图片大小
    ui->prepersonal_label->setPixmap(pixmap);
    ui->postpersonal_label->setPixmap(pixmap);
    ui->check_label->setPixmap(pixmap);
}

void Loading::initSignalSlot() {}

void Loading::startPrePersonal(const QString &duration) {
    // 加载动画
    QMovie *movie = new QMovie(":/image/loading.gif");
    ui->prepersonal_label->setMovie(movie);

    movie->setScaledSize(QSize(32, 32));
    movie->start();
}

void Loading::startPostPersonal(const QString &duration) {

    // 插入图片
    QPixmap pixmap(":/image/success.png");
    ui->prepersonal_label->setPixmap(pixmap);

    // 加载动画
    QMovie *movie = new QMovie(":/image/loading.gif");
    ui->postpersonal_label->setMovie(movie);

    movie->setScaledSize(QSize(32, 32));
    movie->start();

    ui->prepersonal_duration_label->setText(duration);
}

void Loading::startCheck(const QString &duration) {

    // 插入图片
    QPixmap pixmap(":/image/success.png");
    ui->postpersonal_label->setPixmap(pixmap);

    // 加载动画
    QMovie *movie = new QMovie(":/image/loading.gif");
    ui->check_label->setMovie(movie);

    movie->setScaledSize(QSize(32, 32));
    movie->start();

    ui->postpersonal_duration_label->setText(duration);
}

void Loading::finish(const QString &duration) {

    // 插入图片
    QPixmap pixmap(":/image/success.png");
    ui->check_label->setPixmap(pixmap);

    ui->check_duration_label->setText(duration);
}

void Loading::resultReady(const QString &s) { qDebug() << s; }

void Loading::success(Card::Type type, const QString &duration) {

    switch (type) {

    case Card::PREPERSONAL: {
        startPrePersonal(duration);
        break;
    }

    case Card::POSTPERSONAL: {
        startPostPersonal(duration);
        break;
    }

    case Card::CHECK: {
        startCheck(duration);
        break;
    }

    case Card::FINISH: {
        finish(duration);
        break;
    }

    default:
        break;
    }
}