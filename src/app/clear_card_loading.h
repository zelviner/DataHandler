#pragma once
#include "ui_clear_card_loading.h"

#include "task/clear_card.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

class ClearCardLoading : public QMainWindow {
    Q_OBJECT

  public:
    ClearCardLoading(QMainWindow *parent = nullptr);
    ~ClearCardLoading();

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