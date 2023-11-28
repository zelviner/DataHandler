#pragma once

#include "../card_reader.hpp"
#include "pcsc_exception.hpp"

#include <string>
#include <vector>
#include <winscard.h>

class PCSC : public CardReader {
  public:
    PCSC();
    ~PCSC();

    std::vector<std::string> readers() override;

    void connect(int reader_id) override;
    void connect(const std::string &reader_name) override;

    void disconnect() override;

    std::string transmit(const std::string &command) override;

    std::string batch(const std::vector<std::string> &commands) override;

    std::string getATR() override;

    std::string reset(bool cold = true) override;

  private:
    void establishContext();
    void releaseContext();

    std::string       toHexString(const std::vector<BYTE> &data);
    std::vector<BYTE> toBytes(const std::string &data);

  private:
    SCARDCONTEXT             context_;  // 智能卡上下文
    SCARDHANDLE              card_;     // 智能卡句柄
    SCARD_IO_REQUEST         send_pci_; // 发送数据结构
    std::vector<std::string> readers_;  // 读卡器列表
};