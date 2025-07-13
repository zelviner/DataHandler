#pragma once

#include "myorm/model.hpp"
#include "myorm/database.h"

class XhDataloadRecord : public zel::myorm::Model<XhDataloadRecord> {
  public:
    XhDataloadRecord()
        : Model() {}
    XhDataloadRecord(zel::myorm::Database &db)
        : Model(db()) {}
    XhDataloadRecord(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return "xh_dataload_record"; }

    std::string primary_key() const { return "ID"; }
};