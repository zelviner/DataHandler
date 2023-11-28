#pragma once

#include "card_reader.hpp"
#include "pcsc/pcsc.h"

#include <memory>

// 抽象读卡器工厂
class CardReaderFactory {
  public:
    virtual std::unique_ptr<CardReader> createCardReader() = 0;
    virtual ~CardReaderFactory()                           = default;
};

// 具体的 PCSC 读卡器工厂
class PCSCReaderFactory : public CardReaderFactory {
  public:
    std::unique_ptr<CardReader> createCardReader() override { return std::make_unique<PCSC>(); }
};