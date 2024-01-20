#pragma once

#include "info/script.h"

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

#include <repl/repl.h>

#include <json/json.h>
using namespace zel::json;

// 自定义的工作线程类
class WriteCard : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, PREPERSONAL, POSTPERSONAL, CHECK, FINISH };

    WriteCard() {}

    void scriptInfo(ScriptInfo *script_info) { script_info_ = script_info; }

    void jsonData(const Json &json_data) { json_data_ = json_data; }

    void cardReader(std::shared_ptr<CardReader> card_reader) { card_reader_ = card_reader; }

    void readerName(const std::string &reader_name) { reader_name_ = reader_name; }

    void xhlanguageType(int xhlanguage_type) { xhlanguage_type_ = xhlanguage_type; }

    void run() override {

        // 连接读卡器
        card_reader_->connect(reader_name_);

        json_data_["has_ds"] = script_info_->has_ds;
        auto personal_data   = json_data_.str();

        xhlanguage::repl::Repl repl("xhlanguage.log");
        std::string            duration;

        // 获取裸卡 ATR
        auto atr = card_reader_->reset();

        // 预个人化
        emit success(PREPERSONAL, QString::fromStdString(duration), QString::fromStdString(atr));

        // 计时 - 开始
        auto start = std::chrono::steady_clock::now();

        bool success_run = false;
        if (xhlanguage_type_ == 0) {
            success_run = repl.startCompiler(script_info_->clear_buffer, personal_data, card_reader_);
        } else if (xhlanguage_type_ == 1) {
            success_run = repl.startInterpreter(script_info_->clear_buffer, personal_data, card_reader_);
        }

        if (success_run == false) {
            emit failure(PREPERSONAL, QString::fromStdString(""));
            card_reader_->disconnect();
            return;
        }

        // 计时 - 结束
        auto end = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 获取白卡 ATR
        atr = card_reader_->reset();

        // 后个人化
        emit success(POSTPERSONAL, QString::fromStdString(duration), QString::fromStdString(atr));

        // 计时 - 开始
        start = std::chrono::steady_clock::now();

        if (xhlanguage_type_ == 0) {
            success_run = repl.startCompiler(script_info_->clear_buffer, personal_data, card_reader_);
        } else if (xhlanguage_type_ == 1) {
            success_run = repl.startInterpreter(script_info_->clear_buffer, personal_data, card_reader_);
        }

        if (success_run == false) {
            emit failure(POSTPERSONAL, QString::fromStdString(""));
            card_reader_->disconnect();
            return;
        }

        // 计时 - 结束
        end      = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 获取成卡 ATR
        atr = card_reader_->reset();

        // 检测卡片
        emit success(CHECK, QString::fromStdString(duration), QString::fromStdString(atr));

        // 计时 - 开始
        start = std::chrono::steady_clock::now();

        if (xhlanguage_type_ == 0) {
            success_run = repl.startCompiler(script_info_->clear_buffer, personal_data, card_reader_);
        } else if (xhlanguage_type_ == 1) {
            success_run = repl.startInterpreter(script_info_->clear_buffer, personal_data, card_reader_);
        }

        if (success_run == false) {
            emit failure(POSTPERSONAL, QString::fromStdString(""));
            card_reader_->disconnect();
            return;
        }

        // 计时 - 结束
        end      = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 完成
        emit success(FINISH, QString::fromStdString(duration));
        card_reader_->disconnect();
    }

  signals:
    // 信号函数，用于向外界发射信号
    void failure(WriteCard::Type type, const QString &err_msg);

    void success(WriteCard::Type type, const QString &duration, const QString &atr = "");

  private:
    ScriptInfo                 *script_info_;
    Json                        json_data_;
    std::shared_ptr<CardReader> card_reader_;
    std::string                 reader_name_;
    int                         xhlanguage_type_;
};