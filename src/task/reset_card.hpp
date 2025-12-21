#pragma once

#include <qchar.h>
#include <qobjectdefs.h>
#include <qthread.h>
#include <qmetaobject>
#include <qqueue>
#include <zel/core.h>
#include <card_device/card_device.h>

class ResetCard : public QThread {
    Q_OBJECT
  public:
    explicit ResetCard(int reader_id, const DATA_HANDLER &data_handler, QObject *parent = nullptr)
        : QThread(parent)
        , reader_id_(reader_id)
        , data_handler_(data_handler) {}

  signals:
    void resetSuccess(const QString &atr);
    void resetFailure(const QString &err_msg);

  protected:
    void run() override {
        try {
            DH_CardReader(data_handler_, reader_id_);
            char atr[1024];
            DH_ResetCardReader(data_handler_, true, atr, sizeof(atr));
            emit resetSuccess(atr);
        } catch (const std::exception &e) {
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
    DATA_HANDLER data_handler_;
    QString      atr_;
};
