#pragma once
#include "ui_write_card_loading.h"

#include "task/write_card.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

class WriteCardLoading : public QMainWindow {
    Q_OBJECT

  public:
    WriteCardLoading(QMainWindow *parent = nullptr);
    ~WriteCardLoading();

  private:
    // 初始化窗口
    void initWindow();

    // 初始化UI
    void initUI();

    /// @brief 初始化信号槽
    void initSignalSlot();

    void startPrePersonal(const QString &duration);

    void startPostPersonal(const QString &duration);

    void startCheck(const QString &duration);

    void finish(const QString &duration);

  public slots:
    void failure(WriteCard::Type type, const QString &err_msg);

    void success(WriteCard::Type type, const QString &duration);

  private:
    Ui_WriteCardLoading *ui_;
};