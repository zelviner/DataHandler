#include "authenticator.h"
#include "order/script.h"
#include "task/handle_order.hpp"

#include <WinSock2.h>
#include <memory>
#include <qprogressbar>
#include <qpushbutton.h>
#include <qvboxlayout>

Authenticator::Authenticator(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_Authenticator) {
    ui_->setupUi(this);

    init_window();

    init_signals_slots();
}

Authenticator::~Authenticator() { delete ui_; }

void Authenticator::confirmBtnClicked() {
    auto password = ui_->auth_line->text().toStdString();

    emit confirmDeleteOrder(password);
}

void Authenticator::cancelBtnClicked() { emit cancelDeleteOrder(); }

void Authenticator::init_window() {
    qRegisterMetaType<std::shared_ptr<OrderInfo>>("std::shared_ptr<OrderInfo>");
    qRegisterMetaType<std::shared_ptr<PersonDataInfo>>("std::shared_ptr<PersonDataInfo>");
    qRegisterMetaType<std::shared_ptr<ScriptInfo>>("std::shared_ptr<ScriptInfo>");
}

void Authenticator::init_signals_slots() {
    connect(ui_->auth_comfirm_btn, &QPushButton::clicked, this, &Authenticator::confirmBtnClicked);
    connect(ui_->auth_cancel_btn, &QPushButton::clicked, this, &Authenticator::cancelBtnClicked);
}