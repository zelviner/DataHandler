#pragma once

#include "info/script.h"

#include <qcoreapplication>
#include <qdebug>
#include <qthread>
#include <zel/json.h>
#include <xhlanguage/repl.h>


// 自定义的工作线程类
class ClearCard : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, CLEAR, FINISH };

    ClearCard() {}

    void scriptInfo(ScriptInfo *script_info) { script_info_ = script_info; }

    void jsonData(const zel::json::Json &json_data) { json_data_ = json_data; }

    void cardReader(std::shared_ptr<CardReader> card_reader) { card_reader_ = card_reader; }

    void readerName(const std::string &reader_name) { reader_name_ = reader_name; }

    void xhlanguageType(int xhlanguage_type) { xhlanguage_type_ = xhlanguage_type; }

    // 重写run函数，在这里执行线程的工作
    void run() override {

        // 连接读卡器
        card_reader_->connect(reader_name_);

        json_data_["has_ds"] = script_info_->has_ds;
        auto personal_data   = json_data_.str();

        xhlanguage::repl::Repl repl("xhlanguage.log");
        std::string            duration;

        // 清卡
        emit success(CLEAR, QString::fromStdString(duration));

        // 计时 - 开始
        auto start = std::chrono::steady_clock::now();

        bool result = false;
        if (xhlanguage_type_ == 0) {
            result = repl.startCompiler(script_info_->clear_buffer, personal_data, card_reader_);
        } else if (xhlanguage_type_ == 1) {
            result = repl.startInterpreter(script_info_->clear_buffer, personal_data, card_reader_);
        }

        if (result == false) {
            emit failure(CLEAR, QString::fromStdString(""));
            card_reader_->disconnect();
            return;
        }

        // 计时 - 结束
        auto end = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 完成
        emit success(FINISH, QString::fromStdString(duration));
        card_reader_->disconnect();
    }

  signals:
    // 信号函数，用于向外界发射信号
    void failure(ClearCard::Type type, const QString &err_msg);

    void success(ClearCard::Type type, const QString &duration);

  private:
    ScriptInfo                 *script_info_;
    zel::json::Json             json_data_;
    std::shared_ptr<CardReader> card_reader_;
    std::string                 reader_name_;
    int                         xhlanguage_type_;
};