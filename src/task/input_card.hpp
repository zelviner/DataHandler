#pragma once

#include "card.h"

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

// 自定义的工作线程类
class InputCard : public QThread {
    Q_OBJECT

  signals:
    // 信号函数，用于向外界发射信号
    void resultReady(const QString &s);

    void success(const int a);

  public:
    InputCard(std::shared_ptr<Card> card)
        : card_(card) {}
    void run() override {
        // 连接读卡器
        if (!card_->connectCard()) {
            emit resultReady("connect card failed");
            return;
        }

        // 预个人化
        if (!card_->prePersonal()) {
            card_->disconnectCard();
            emit resultReady("prepersonal card failed");
            return;
        }
        emit success(1);

        // 后个人化
        if (!card_->postPersonal()) {
            card_->disconnectCard();
            emit resultReady("postpersonal card failed");
            return;
        }
        emit success(2);

        // 检测卡片
        if (!card_->checkCard()) {
            card_->disconnectCard();
            emit resultReady("check card failed");
            return;
        }
        emit success(3);

        // 断开连接
        card_->disconnectCard();

        emit resultReady("input card success");
    }

  private:
    std::shared_ptr<Card> card_;
};