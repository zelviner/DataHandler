#pragma once

#include "task/aka_auth.hpp"
#include "ui_aka_auth_loading.h"

#include <qapplication>
#include <qmainwindow>
#include <qpushbutton>

class AkaAuthLoading : public QMainWindow {
    Q_OBJECT

  public:
    AkaAuthLoading(QMainWindow *parent = nullptr);
    ~AkaAuthLoading();

  private:
    // 初始化窗口
    void init_window();

    // 初始化UI
    void init_ui();

    /// @brief 初始化信号槽
    void init_signal_slot();

    void start_auth();

    void auth(const QString &apdu_response);

    void finish(const QString &duration);

  public slots:
    void failure(AkaAuth::Type type, const QString &err_msg);
    void success(AkaAuth::Type type, const QString &duration, const QString &apdu_response);

  private:
    Ui_AkaAuthLoading *ui_;
};