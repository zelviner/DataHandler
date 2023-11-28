#pragma once

#include <stdexcept>

class PCSCException : public std::exception {
  public:
    PCSCException(long error)
        : errorCode(error) {}

    const char *what() const noexcept override { return "PC/SC Error"; }

    long getErrorCode() const { return errorCode; }

  private:
    long errorCode;
};
