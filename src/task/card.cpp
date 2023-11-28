#include "card.h"

#include "card-reader/card_reader_factory.hpp"
#include "object/object.h"
using namespace xhlanguage::interpreter;

Card::Card(ScriptInfo *script_info, const std::string &personal_data)
    : interpreter_(new Interpreter)
    , script_info_(script_info)
    , personal_data_(personal_data) {}

Card::~Card() {}

bool Card::connectCard() {

    // 创建 PCSC 读卡器工厂
    std::unique_ptr<CardReaderFactory> card_reader_factory = std::make_unique<PCSCReaderFactory>();

    // 使用工厂创建 PCSC 读卡器
    card_reader_ = card_reader_factory->createCardReader();

    try {
        // 连接读卡器
        card_reader_->connect(0);
    } catch (std::exception &e) {
        std::cout << "connect card reader error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool Card::prePersonal() {

    // 计时 - 开始
    auto start = std::chrono::steady_clock::now();

    auto result = interpreter_->interpret(script_info_->person_buffer, "", card_reader_);

    // 计时 - 结束
    auto        end          = std::chrono::steady_clock::now();
    auto        duration     = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::string duration_str = std::to_string(duration.count() * 1000) + "ms";

    if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
        std::cout << "script interpreter error: " << std::endl;
        std::cout << result->inspect() << std::endl;
        std::cout << "duration: " << duration_str << std::endl;
        return false;
    }

    std::cout << "script run success, duration: " << duration_str << std::endl;
    return true;
}

bool Card::postPersonal() {

    // 计时 - 开始
    auto start = std::chrono::steady_clock::now();

    auto result = interpreter_->interpret(script_info_->post_person_buffer, personal_data_, card_reader_);

    // 计时 - 结束
    auto        end          = std::chrono::steady_clock::now();
    auto        duration     = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::string duration_str = std::to_string(duration.count() * 1000) + "ms";

    if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
        std::cout << "script interpreter error: " << std::endl;
        std::cout << result->inspect() << std::endl;
        std::cout << "duration: " << duration_str << std::endl;
        return false;
    }

    std::cout << "script run success, duration: " << duration_str << std::endl;
    return true;
}

bool Card::checkCard() {

    // 计时 - 开始
    auto start = std::chrono::steady_clock::now();

    auto result = interpreter_->interpret(script_info_->check_buffer, personal_data_, card_reader_);

    // 计时 - 结束
    auto        end          = std::chrono::steady_clock::now();
    auto        duration     = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::string duration_str = std::to_string(duration.count() * 1000) + "ms";

    if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
        std::cout << "script interpreter error: " << std::endl;
        std::cout << result->inspect() << std::endl;
        std::cout << "duration: " << duration_str << std::endl;
        return false;
    }

    std::cout << "script run success, duration: " << duration_str << std::endl;
    return true;
}

bool Card::clearCard() {
    // 计时 - 开始
    auto start = std::chrono::steady_clock::now();

    auto result = interpreter_->interpret(script_info_->clear_buffer, "", card_reader_);

    // 计时 - 结束
    auto        end          = std::chrono::steady_clock::now();
    auto        duration     = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::string duration_str = std::to_string(duration.count() * 1000) + "ms";

    if (result->type() == xhlanguage::object::Object::OBJECT_ERROR) {
        std::cout << "script interpreter error: " << std::endl;
        std::cout << result->inspect() << std::endl;
        std::cout << "duration: " << duration_str << std::endl;
        return false;
    }

    std::cout << "script run success, duration: " << duration_str << std::endl;
    return true;
}

void Card::disconnectCard() { card_reader_->disconnect(); }