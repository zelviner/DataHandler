#pragma once

#include <zel/myorm.h>

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