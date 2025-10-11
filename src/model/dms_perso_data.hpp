#pragma once

#include "myorm/model.hpp"
#include "myorm/database.h"

class DmsPersoData : public zel::myorm::Model<DmsPersoData> {
  public:
    DmsPersoData()
        : Model() {}
    DmsPersoData(zel::myorm::Database &db, const std::string &table_name, const std::string &primary_key)
        : Model(db())
        , table_name_(table_name)
        , primary_key_(primary_key) {}
    DmsPersoData(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return table_name_; }

    std::string primary_key() const { return primary_key_; }

  private:
    std::string table_name_;
    std::string primary_key_;
};