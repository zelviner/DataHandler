#pragma once

#include "card-reader/card_reader_factory.hpp"
#include "info/script.h"

#include <interpreter/interpreter.h>
using namespace xhlanguage::interpreter;

#include <json/json.h>
using namespace zel::json;

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

// 自定义的工作线程类
class ClearCard : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, CLEAR, FINISH };

    ClearCard() {}

    void scriptInfo(ScriptInfo *script_info) { script_info_ = script_info; }

    void jsonData(const Json &json_data) { json_data_ = json_data; }

    void cardReader(std::shared_ptr<CardReader> card_reader) { card_reader_ = card_reader; }


    // 重写run函数，在这里执行线程的工作
    void run() override {

        json_data_["has_ds"] = script_info_->has_ds;
        auto personal_data   = json_data_.str();

        // 创建脚本解释器
        Interpreter interpreter;
        std::string duration;

        // 清卡
        emit success(CLEAR, QString::fromStdString(duration));

        // 计时 - 开始
        auto start = std::chrono::steady_clock::now();
        if (!interpreter.interpret(script_info_->clear_buffer, personal_data, card_reader_)) {
            emit failure(CLEAR, QString::fromStdString(""));
            return;
        }

        // 计时 - 结束
        auto end = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 完成
        emit success(FINISH, QString::fromStdString(duration));
    }

  signals:
    // 信号函数，用于向外界发射信号
    void failure(ClearCard::Type type, const QString &err_msg);

    void success(ClearCard::Type type, const QString &duration);

  private:
    ScriptInfo                 *script_info_;
    Json                        json_data_;
    std::shared_ptr<CardReader> card_reader_;
};