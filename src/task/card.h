#pragma once

#include "info/person_data.h"
#include "info/script.h"

#include <interpreter/interpreter.h>
#include <memory>

class Card {
  public:
    Card(ScriptInfo *script_info, const std::string &personal_data);
    ~Card();

    /// @brief 连接卡片
    bool connectCard();

    /// @brief 预个人化
    bool prePersonal();

    /// @brief 后个人化
    bool postPersonal();

    /// @brief 检测卡片
    bool checkCard();

    /// @brief 清卡
    bool clearCard();

    /// @brief 断开卡片
    void disconnectCard();

  private:
    std::shared_ptr<CardReader>                           card_reader_;
    std::shared_ptr<xhlanguage::interpreter::Interpreter> interpreter_;
    ScriptInfo                                           *script_info_;
    std::string                                           personal_data_;
};