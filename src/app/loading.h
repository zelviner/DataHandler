#pragma once

#include "ui_loading.h"

#include <qapplication>
#include <qmainwindow.h>
#include <qmovie>
#include <qpushbutton>

class Loading : public QMainWindow {
    Q_OBJECT

  public:
    Loading(QMainWindow *parent = nullptr);
    ~Loading();

  private:
    void init_window();

  private:
    Ui_Loading *ui_;

};