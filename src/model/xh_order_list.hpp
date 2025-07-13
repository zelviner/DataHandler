#pragma once

#include "myorm/model.hpp"
#include "myorm/database.h"

class XhOrderList : public zel::myorm::Model<XhOrderList> {
  public:
    XhOrderList()
        : Model() {}
    XhOrderList(zel::myorm::Database &db)
        : Model(db()) {}
    XhOrderList(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return "xh_order_list"; }

    std::string primary_key() const { return "ID"; }
};