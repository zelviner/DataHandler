#include "pcsc.h"

PCSC::PCSC() { establishContext(); }

PCSC::~PCSC() { releaseContext(); }

std::vector<std::string> PCSC::readers() {

    DWORD readersSize = 0;
    LONG  result      = SCardListReaders(context_, nullptr, nullptr, &readersSize);
    if (result != SCARD_S_SUCCESS) {
        throw PCSCException(result);
    }

    std::vector<char> readersBuffer(readersSize, 0);
    result = SCardListReaders(context_, nullptr, readersBuffer.data(), &readersSize);
    if (result != SCARD_S_SUCCESS) {
        throw PCSCException(result);
    }

    char *reader = readersBuffer.data();
    while (*reader != '\0') {
        readers_.push_back(reader);
        reader += strlen(reader) + 1;
    }

    return readers_;
}

void PCSC::connect(int reader_id) {
    if (readers_.empty()) readers();

    if (reader_id < 0 || reader_id >= readers_.size()) {
        throw std::runtime_error("Invalid reader ID");
    }

    connect(readers_[reader_id]);
}

void PCSC::connect(const std::string &reader_name) {
    DWORD active_protocol = 0;
    LONG result = SCardConnect(context_, reader_name.c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                               &card_, &active_protocol);
    if (result != SCARD_S_SUCCESS) {
        throw PCSCException(result);
    }

    send_pci_.dwProtocol  = active_protocol;
    send_pci_.cbPciLength = sizeof(send_pci_);
}

void PCSC::disconnect() {
    if (card_ != NULL) {
        SCardDisconnect(card_, SCARD_LEAVE_CARD);
        card_ = NULL;
    }
}

std::string PCSC::transmit(const std::string &command) {
    std::vector<BYTE> bytes_command = toBytes(command);
    std::vector<BYTE> response(256, 0); // Adjust the size as needed
    DWORD             responseSize = static_cast<DWORD>(response.size());

    LONG result = SCardTransmit(card_, &send_pci_, bytes_command.data(), static_cast<DWORD>(bytes_command.size()),
                                nullptr, response.data(), &responseSize);
    if (result != SCARD_S_SUCCESS) {
        throw PCSCException(result);
    }

    response.resize(responseSize);

    return toHexString(response);
}

std::string PCSC::batch(const std::vector<std::string> &commands) {
    std::string result;
    for (const std::string &command : commands) {
        result += transmit(command);
    }
    return result;
}

// atr
std::string PCSC::getATR() {
    std::vector<BYTE> atr(256, 0); // Adjust the size as needed
    DWORD             atrSize = static_cast<DWORD>(atr.size());

    LONG result = SCardGetAttrib(card_, SCARD_ATTR_ATR_STRING, atr.data(), &atrSize);
    if (result != SCARD_S_SUCCESS) {
        throw PCSCException(result);
    }

    atr.resize(atrSize);
    return toHexString(atr);
}

// 复位， 包括冷复位和热复位
std::string PCSC::reset(bool cold) {
    LONG result = SCardReconnect(card_, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                                 cold ? SCARD_UNPOWER_CARD : SCARD_RESET_CARD, &send_pci_.dwProtocol);
    if (result != SCARD_S_SUCCESS) {
        throw PCSCException(result);
    }

    DWORD             atr_len = 40;
    std::vector<BYTE> atr(atr_len, 0);
    result = SCardStatus(card_, nullptr, nullptr, nullptr, nullptr, atr.data(), &atr_len);
    if (result != SCARD_S_SUCCESS) {
        throw PCSCException(result);
    }

    atr.resize(atr_len);
    return toHexString(atr);
}

void PCSC::establishContext() {
    LONG result = SCardEstablishContext(SCARD_SCOPE_SYSTEM, nullptr, nullptr, &context_);
    if (result != SCARD_S_SUCCESS) {
        throw PCSCException(result);
    }
}

void PCSC::releaseContext() {
    if (context_ != 0) {
        SCardReleaseContext(context_);
        context_ = 0;
    }
}

std::string PCSC::toHexString(const std::vector<BYTE> &data) {
    std::string result;
    for (BYTE byte : data) {
        char buf[3];
        sprintf(buf, "%02X", byte);
        result += buf;
    }
    return result;
}

std::vector<BYTE> PCSC::toBytes(const std::string &data) {
    std::vector<BYTE> result;
    for (size_t i = 0; i < data.size(); i += 2) {
        result.push_back(std::stoi(data.substr(i, 2), nullptr, 16));
    }
    return result;
}