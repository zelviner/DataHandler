#pragma once

#include <zel/myorm.h>

class DmsRemakeRecord : public zel::myorm::Model<DmsRemakeRecord> {
  public:
    DmsRemakeRecord()
        : Model() {}
    DmsRemakeRecord(zel::myorm::Database &db, const std::string &order_no, const std::string &primary_key)
        : Model(db())
        , order_no_(order_no)
        , primary_key_(primary_key) {}
    DmsRemakeRecord(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return order_no_ + "_remakerecord"; }

    std::string primary_key() const { return primary_key_; }

  private:
    std::string order_no_;
    std::string primary_key_;
};