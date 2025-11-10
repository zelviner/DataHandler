#pragma once

#include <zel/myorm.h>

class DmsDeleteRecord : public zel::myorm::Model<DmsDeleteRecord> {
  public:
    DmsDeleteRecord()
        : Model() {}
    DmsDeleteRecord(zel::myorm::Database &db)
        : Model(db()) {}
    DmsDeleteRecord(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return "dms_deleterecord"; }

    std::string primary_key() const { return "ID"; }
};