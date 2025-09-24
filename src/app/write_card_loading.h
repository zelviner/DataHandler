#pragma once

#include "task/write_card.hpp"
#include "ui_write_card_loading.h"

#include <qapplication>
#include <qmainwindow>
#include <qpushbutton>

class WriteCardLoading : public QMainWindow {
    Q_OBJECT

  public:
    WriteCardLoading(QMainWindow *parent = nullptr);
    ~WriteCardLoading();

  signals:
    // 信号函数，用于向外界发射信号
    void bareAtr(const QString &bare_atr);
    void whiteAtr(const QString &white_atr);
    void finishedAtr(const QString &finished_atr);

  private:
    // 初始化窗口
    void initWindow();

    // 初始化UI
    void initUI();

    /// @brief 初始化信号槽
    void initSignalSlot();

    void startPrePersonal(const QString &duration, const QString &atr);

    void prePersonal(const QString &duration, const QString &apdu_response);

    void startPostPersonal(const QString &duration, const QString &atr);

    void postPersonal(const QString &duration, const QString &apdu_response);

    void startCheck(const QString &duration, const QString &atr);

    void check(const QString &duration, const QString &apdu_response);

    void finish(const QString &duration);

  public slots:
    void failure(WriteCard::Type type, const QString &err_msg);
    void success(WriteCard::Type type, const QString &duration, const QString &apdu_response = "");

  private:
    Ui_WriteCardLoading *ui_;
};