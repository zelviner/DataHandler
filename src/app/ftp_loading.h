#pragma once

#include "ui_ftp_loading.h"
#include "task/upload_prd.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

class FtpLoading : public QMainWindow {
    Q_OBJECT

  public:
    FtpLoading(QMainWindow *parent = nullptr);
    ~FtpLoading();

  private:
    // 初始化窗口
    void initWindow();

    // 初始化UI
    void initUI();

    /// @brief 初始化信号槽
    void initSignalSlot();

    void startUpload(const QString &duration);

    void finish(const QString &duration);

  private slots:
    void failure(UploadPrd::Type type, const QString &err_msg);

  private:
    Ui_FtpLoading *ui_;
};