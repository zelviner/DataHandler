#pragma once

#include "card-reader/card_reader_factory.hpp"
#include "info/script.h"

#include <interpreter/interpreter.h>
using namespace xhlanguage::interpreter;

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

    void personalData(const std::string &personal_data) { personal_data_ = personal_data; }

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

        // 创建脚本解释器
        Interpreter interpreter;

        std::string duration;
        // 预个人化
        emit success(PREPERSONAL, QString::fromStdString(duration));

        // 计时 - 开始
        auto start = std::chrono::steady_clock::now();

        auto result = interpreter.interpret(script_info_->person_buffer, "", card_reader);
        if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
            std::cout << "script interpreter error: " << std::endl;
            std::cout << result->inspect() << std::endl;
            card_reader->disconnect();
            emit failure(PREPERSONAL, QString::fromStdString(result->inspect()));
            return;
        }

        // 计时 - 结束
        auto end = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 后个人化
        emit success(POSTPERSONAL, QString::fromStdString(duration));

        // 计时 - 开始
        start = std::chrono::steady_clock::now();

        result = interpreter.interpret(script_info_->post_person_buffer, personal_data_, card_reader);
        if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
            std::cout << "script interpreter error: " << std::endl;
            std::cout << result->inspect() << std::endl;
            card_reader->disconnect();
            emit failure(POSTPERSONAL, QString::fromStdString(result->inspect()));
            return;
        }

        // 计时 - 结束
        end      = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 检测卡片
        emit success(CHECK, QString::fromStdString(duration));

        // 计时 - 开始
        start = std::chrono::steady_clock::now();

        result = interpreter.interpret(script_info_->check_buffer, personal_data_, card_reader);
        if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
            std::cout << "script interpreter error: " << std::endl;
            std::cout << result->inspect() << std::endl;
            card_reader->disconnect();
            emit failure(POSTPERSONAL, QString::fromStdString(result->inspect()));
            return;
        }

        // 计时 - 结束
        end      = std::chrono::steady_clock::now();
        duration = "用时: " + std::to_string(std::chrono::duration<double>(end - start).count()) + " 秒";

        // 完成
        emit success(FINISH, QString::fromStdString(duration));

        // 断开连接
        card_reader->disconnect();
    }

  signals:
    // 信号函数，用于向外界发射信号
    void failure(WriteCard::Type type, const QString &err_msg);

    void success(WriteCard::Type type, const QString &duration);

  private:
    ScriptInfo *script_info_;
    std::string personal_data_;
};