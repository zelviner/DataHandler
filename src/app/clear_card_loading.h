#pragma once

#include "task/clear_card.hpp"
#include "ui_clear_card_loading.h"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

class ClearCardLoading : public QMainWindow {
    Q_OBJECT

  public:
    ClearCardLoading(QMainWindow *parent = nullptr);
    ~ClearCardLoading();

  signals:
    // 信号函数，用于向外界发射信号
    void atr(const QString &atr);

  private:
    // 初始化窗口
    void initWindow();

    // 初始化UI
    void initUI();

    /// @brief 初始化信号槽
    void initSignalSlot();

    void startClear(const QString &duration);

    void finish(const QString &duration);

  public slots:
    void failure(ClearCard::Type type, const QString &err_msg);

    void success(ClearCard::Type type, const QString &duration);

  private:
    Ui_ClearCardLoading *ui_;
};