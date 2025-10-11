#pragma once

#include "myorm/model.hpp"
#include "myorm/database.h"

class DmsRemake : public zel::myorm::Model<DmsRemake> {
  public:
    DmsRemake()
        : Model() {}
    DmsRemake(zel::myorm::Database &db, const std::string &order_no, const std::string &primary_key)
        : Model(db())
        , order_no_(order_no)
        , primary_key_(primary_key) {}
    DmsRemake(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return order_no_ + "_remake"; }

    std::string primary_key() const { return primary_key_; }

  private:
    std::string order_no_;
    std::string primary_key_;
};