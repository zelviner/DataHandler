#pragma once

#include <string>
#include <vector>

class CardReader {

  public:
    CardReader(){};
    ~CardReader(){};

    virtual std::vector<std::string> readers()                                       = 0;
    virtual void                     connect(int reader_id)                          = 0;
    virtual void                     connect(const std::string &reader_name)         = 0;
    virtual void                     disconnect()                                    = 0;
    virtual std::string              transmit(const std::string &command)            = 0;
    virtual std::string              batch(const std::vector<std::string> &commands) = 0;
    virtual std::string              getATR()                                        = 0;
    virtual std::string              reset(bool cold = true)                         = 0;
};