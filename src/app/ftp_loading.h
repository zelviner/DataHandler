#pragma once

#include "ui_ftp_loading.h"

#include <qapplication>
#include <qwidget>
#include <qmovie>
#include <qpushbutton>

class FtpLoading : public QWidget {
    Q_OBJECT

  public:
    FtpLoading(QWidget *parent = nullptr);
    ~FtpLoading();

  private:
    Ui_FtpLoading *ui_;
    QMovie        *movie_;
};