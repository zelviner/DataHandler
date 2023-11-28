#pragma once

#include "card.h"

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

// 自定义的工作线程类
class InputCard : public QThread {
    Q_OBJECT

  public:
    InputCard(std::shared_ptr<Card> card)
        : card_(card) {}

    void run() override {
        // 连接读卡器
        if (!card_->connectCard()) {
            emit resultReady("connect card failed");
            return;
        }

        std::string duration;
        // 预个人化
        emit success(Card::PREPERSONAL, QString::fromStdString(duration));
        if (!card_->prePersonal(duration)) {
            card_->disconnectCard();
            emit resultReady("prepersonal card failed");
            return;
        }

        // 后个人化
        emit success(Card::POSTPERSONAL, QString::fromStdString(duration));
        if (!card_->postPersonal(duration)) {
            card_->disconnectCard();
            emit resultReady("postpersonal card failed");
            return;
        }

        // 检测卡片
        emit success(Card::CHECK, QString::fromStdString(duration));
        if (!card_->checkCard(duration)) {
            card_->disconnectCard();
            emit resultReady("check card failed");
            return;
        }

        // 完成
        emit success(Card::FINISH, QString::fromStdString(duration));

        // 断开连接
        card_->disconnectCard();

        emit resultReady("input card success");
    }

  signals:
    // 信号函数，用于向外界发射信号
    void resultReady(const QString &s);

    void success(Card::Type type, const QString &duration);

  private:
    std::shared_ptr<Card> card_;
};