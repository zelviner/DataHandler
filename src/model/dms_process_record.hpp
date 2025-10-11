#pragma once

#include "myorm/model.hpp"
#include "myorm/database.h"

class DmsProcessRecord : public zel::myorm::Model<DmsProcessRecord> {
  public:
    DmsProcessRecord()
        : Model() {}
    DmsProcessRecord(zel::myorm::Database &db)
        : Model(db()) {}
    DmsProcessRecord(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return "dms_processrecord"; }

    std::string primary_key() const { return "ID"; }
};