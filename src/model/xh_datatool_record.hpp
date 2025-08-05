#pragma once

#include "myorm/model.hpp"
#include "myorm/database.h"

class XhDatatoolRecord : public zel::myorm::Model<XhDatatoolRecord> {
  public:
    XhDatatoolRecord()
        : Model() {}
    XhDatatoolRecord(zel::myorm::Database &db)
        : Model(db()) {}
    XhDatatoolRecord(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return "xh_datatool_record"; }

    std::string primary_key() const { return "ID"; }
};