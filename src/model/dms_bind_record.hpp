#pragma once

#include <zel/myorm.h>

class DmsBindRecord : public zel::myorm::Model<DmsBindRecord> {
  public:
    DmsBindRecord()
        : Model() {}
    DmsBindRecord(zel::myorm::Database &db)
        : Model(db()) {}
    DmsBindRecord(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return "dms_bindrecord"; }

    std::string primary_key() const { return "ID"; }
};