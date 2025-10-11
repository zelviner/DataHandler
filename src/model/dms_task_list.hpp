#pragma once

#include "myorm/model.hpp"
#include "myorm/database.h"

class DmsTaskList : public zel::myorm::Model<DmsTaskList> {
  public:
    DmsTaskList()
        : Model() {}
    DmsTaskList(zel::myorm::Database &db)
        : Model(db()) {}
    DmsTaskList(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return "dms_tasklist"; }

    std::string primary_key() const { return "ID"; }
};