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
class ClearCard : public QThread {
    Q_OBJECT

  public:
    enum Type { CONNECT, CLEAR, FINISH };

    ClearCard() {}

    void scriptInfo(ScriptInfo *script_info) { script_info_ = script_info; }

    void jsonData(const Json &json_data) { json_data_ = json_data; }

    // 重写run函数，在这里执行线程的工作
    void run() override {

        // 创建 PCSC 读卡器工厂
        std::unique_ptr<CardReaderFactory> card_reader_factory = std::make_unique<PCSCReaderFactory>();

        // 使用工厂创建 PCSC 读卡器
        auto card_reader = card_reader_factory->createCardReader();

        try {
            // 连接读卡器
            card_reader->connect(0);
        } catch (std::exception &e) {
            QString err_msg = "connect card reader error: " + QString::fromStdString(e.what());
            emit    failure(CONNECT, err_msg);
            return;
        }

        json_data_["has_ds"] = script_info_->has_ds;
        auto personal_data   = json_data_.str();

        // 创建脚本解释器
        Interpreter interpreter;

        std::string duration;

        // 清卡
        emit success(CLEAR, QString::fromStdString(duration));

        // 计时 - 开始
        auto start  = std::chrono::steady_clock::now();
        auto result = interpreter.interpret(script_info_->clear_buffer, personal_data, card_reader);
        if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
            std::cout << "script interpreter error: " << std::endl;
            std::cout << result->inspect() << std::endl;
            card_reader->disconnect();
            emit failure(CLEAR, QString::fromStdString(result->inspect()));
            return;
        }

        // 计时 - 结束
        auto end = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 完成
        emit success(FINISH, QString::fromStdString(duration));

        // 断开连接
        card_reader->disconnect();
    }

  signals:
    // 信号函数，用于向外界发射信号
    void failure(ClearCard::Type type, const QString &err_msg);

    void success(ClearCard::Type type, const QString &duration);

  private:
    ScriptInfo *script_info_;
    Json        json_data_;
};