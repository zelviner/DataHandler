#pragma once

#include "card.h"
#include <memory>

#include <QCoreApplication>
#include <QDebug>
#include <QThread>


// 自定义的工作线程类
class ClearCard : public QThread {
    Q_OBJECT

  signals:
    // 信号函数，用于向外界发射信号
    void resultReady(const QString &s);

  public:
    // 构造函数，接受一个参数
    ClearCard(std::shared_ptr<Card> card)
        : card_(card) {}

    // 重写run函数，在这里执行线程的工作
    void run() override {

        // 连接读卡器
        if (!card_->connectCard()) {
            emit resultReady("connect card failed");
            return;
        }

        // 清卡
        if (!card_->clearCard()) {
            card_->disconnectCard();
            emit resultReady("clear card failed");
            return;
        }

        // 断开连接
        card_->disconnectCard();

        emit resultReady("clear card success");
    }

  private:
    std::shared_ptr<Card> card_;
};