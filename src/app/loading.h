#pragma once
#include "ui_loading.h"

#include "info/person_data.h"
#include "info/script.h"
#include "task/card.h"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

class Loading : public QMainWindow {
    Q_OBJECT

  public:
    Loading(ScriptInfo *script_info, const std::string &personal_data, QMainWindow *parent = nullptr);
    ~Loading();

    bool inputCard();

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

  private slots:
    void resultReady(const QString &s);

    void success(Card::Type type, const QString &duration);

  private:
    Ui_Loading *ui;
    ScriptInfo *script_info_;
    std::string personal_data_;
};