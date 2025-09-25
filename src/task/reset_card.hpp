#pragma once

#include <qchar.h>
#include <qobjectdefs.h>
#include <qthread.h>
#include <xhlanguage/repl/repl_bridge.h>
#include <qmetaobject>
#include <QCoreApplication>
#include <qqueue>
#include <zel/utility/logger.h>

class ResetCard : public QThread {
    Q_OBJECT
  public:
    explicit ResetCard(int reader_id, QObject *parent = nullptr)
        : QThread(parent)
        , reader_id_(reader_id) {}

  signals:
    void resetSuccess(const QString &atr);
    void resetFailure(const QString &err_msg);

  protected:
    void run() override {
        setCallback(&ResetCard::callbackThunk, this);

        char error_message[1024];
        if (!startCompiler("RST()", "", reader_id_, error_message, sizeof(error_message))) {
            emit resetFailure(error_message);
        }
    }

  private:
    static void callbackThunk(const char *run_result, int len, void *user) {
        auto *self = static_cast<ResetCard *>(user);
        self->atr_ = QString::fromUtf8(run_result, len); // 保存结果
        emit self->resetSuccess(self->atr_);
    }

  private:
    int     reader_id_;
    QString atr_;
};
