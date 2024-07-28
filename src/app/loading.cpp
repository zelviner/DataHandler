#include "loading.h"
#include "order/script.h"
#include "task/handle_order.hpp"

#include <memory>
#include <qprogressbar>
#include <qvboxlayout>

Loading::Loading(QMainWindow *parent)
    : QMainWindow(parent)
    , ui_(new Ui_Loading) {
    ui_->setupUi(this);

    ui_->loading_progress->setRange(0, 0); // Set range to 0 to enable indeterminate state

    initWindow();
}

Loading::~Loading() { delete ui_; }

void Loading::initWindow() {
    qRegisterMetaType<std::shared_ptr<OrderInfo>>("std::shared_ptr<OrderInfo>");
    qRegisterMetaType<std::shared_ptr<PersonDataInfo>>("std::shared_ptr<PersonDataInfo>");
    qRegisterMetaType<std::shared_ptr<ScriptInfo>>("std::shared_ptr<ScriptInfo>");
}