#pragma once

#include "ui_order_window.h"
#include "order-info/order.h"

#include <zel/filesystem.h>
#include <memory>
#include <qmainwindow>

class OrderWindow : public QMainWindow {
    Q_OBJECT

  public:
    OrderWindow(const std::string &datagram_path, QMainWindow *parent = nullptr);
    ~OrderWindow();

    /// @brief 确定按钮点击事件
    void confirmBtnClicked();

    /// @brief 取消按钮点击事件
    void cancelBtnClicked();

  signals:
    void confirmOrder(const std::string &confirm_datagram_dir);
    void cancelOrder();

  private:
    /// @brief 初始化窗口
    void initWindow();

    /// @brief 初始化UI
    void initUI();

    /// @brief 初始化信号槽
    void initSignalSlot();

  private:
    std::shared_ptr<Ui_OrderWindow>        ui_;
    std::unique_ptr<zel::filesystem::File> datagram_file_;
    std::vector<std::string>               datagram_format_;
};