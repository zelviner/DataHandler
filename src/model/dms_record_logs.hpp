#pragma once

#include "myorm/model.hpp"
#include "myorm/database.h"

class DmsRecordLogs : public zel::myorm::Model<DmsRecordLogs> {
  public:
    DmsRecordLogs()
        : Model() {}
    DmsRecordLogs(zel::myorm::Database &db, const std::string &order_no, const std::string &primary_key)
        : Model(db())
        , order_no_(order_no)
        , primary_key_(primary_key) {}
    DmsRecordLogs(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return order_no_ + "_record_logs"; }

    std::string primary_key() const { return primary_key_; }

  private:
    std::string order_no_;
    std::string primary_key_;
};