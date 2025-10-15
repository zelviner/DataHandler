#pragma once

#include <qchar.h>
#include <qobjectdefs.h>
#include <qthread.h>
#include <if_language/repl/repl_bridge.h>
#include <qmetaobject>
#include <qqueue>
#include <zel/utility/logger.h>

class ResetCard : public QThread {
    Q_OBJECT
  public:
    explicit ResetCard(int reader_id, ApduProtocol protocol, QObject *parent = nullptr)
        : QThread(parent)
        , reader_id_(reader_id)
        , protocol_(protocol) {}

  signals:
    void resetSuccess(const QString &atr);
    void resetFailure(const QString &err_msg);

  protected:
    void run() override {
        setCallback(&ResetCard::callback_thunk, this);

        if (!startCompiler("RST()", "", reader_id_, protocol_)) {
            emit resetFailure("复位卡片失败，请检查读卡器是否连接正确。");
        }
    }

  private:
    static void callback_thunk(const char *run_result, int len, void *user) {
        auto *self = static_cast<ResetCard *>(user);
        self->atr_ = QString::fromUtf8(run_result, len); // 保存结果
        emit self->resetSuccess(self->atr_);
    }

  private:
    int          reader_id_;
    ApduProtocol protocol_;
    QString      atr_;
};
