#pragma once

#include "card-reader/card_reader_factory.hpp"
#include "info/script.h"

#include <interpreter/interpreter.h>
using namespace xhlanguage::interpreter;

#include <public/json/json.h>
using namespace zel::json;

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

// 自定义的工作线程类
class WriteCard : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, PREPERSONAL, POSTPERSONAL, CHECK, FINISH };

    WriteCard() {}

    void scriptInfo(ScriptInfo *script_info) { script_info_ = script_info; }

    void jsonData(const Json &json_data) { json_data_ = json_data; }

    void cardReader(std::shared_ptr<CardReader> card_reader) { card_reader_ = card_reader; }

    void run() override {

        json_data_["has_ds"] = script_info_->has_ds;
        auto personal_data   = json_data_.str();

        // 创建脚本解释器
        Interpreter interpreter;

        std::string duration, atr;

        // 获取裸卡 ATR
        auto result = interpreter.interpret("RST()", "", card_reader_);
        atr         = result->inspect().substr(10);

        // 预个人化
        emit success(PREPERSONAL, QString::fromStdString(duration), QString::fromStdString(atr));

        // 计时 - 开始
        auto start = std::chrono::steady_clock::now();

        result = interpreter.interpret(script_info_->person_buffer, "", card_reader_);
        if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
            std::cout << "script interpreter error: " << std::endl;
            std::cout << result->inspect() << std::endl;
            card_reader_->disconnect();
            emit failure(PREPERSONAL, QString::fromStdString(result->inspect()));
            return;
        }

        // 计时 - 结束
        auto end = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 获取白卡 ATR
        result = interpreter.interpret("RST()", "", card_reader_);
        atr    = result->inspect().substr(10);

        // 后个人化
        emit success(POSTPERSONAL, QString::fromStdString(duration), QString::fromStdString(atr));

        // 计时 - 开始
        start = std::chrono::steady_clock::now();

        result = interpreter.interpret(script_info_->post_person_buffer, personal_data, card_reader_);
        if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
            std::cout << "script interpreter error: " << std::endl;
            std::cout << result->inspect() << std::endl;
            card_reader_->disconnect();
            emit failure(POSTPERSONAL, QString::fromStdString(result->inspect()));
            return;
        }

        // 计时 - 结束
        end      = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 获取成卡 ATR
        result = interpreter.interpret("RST()", "", card_reader_);
        atr    = result->inspect().substr(10);

        // 检测卡片
        emit success(CHECK, QString::fromStdString(duration), QString::fromStdString(atr));

        // 计时 - 开始
        start = std::chrono::steady_clock::now();

        result = interpreter.interpret(script_info_->check_buffer, personal_data, card_reader_);
        if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
            std::cout << "script interpreter error: " << std::endl;
            std::cout << result->inspect() << std::endl;
            card_reader_->disconnect();
            emit failure(POSTPERSONAL, QString::fromStdString(result->inspect()));
            return;
        }

        // 计时 - 结束
        end      = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 完成
        emit success(FINISH, QString::fromStdString(duration));
    }

  signals:
    // 信号函数，用于向外界发射信号
    void failure(WriteCard::Type type, const QString &err_msg);

    void success(WriteCard::Type type, const QString &duration, const QString &atr = "");

  private:
    ScriptInfo                 *script_info_;
    Json                        json_data_;
    std::shared_ptr<CardReader> card_reader_;
};