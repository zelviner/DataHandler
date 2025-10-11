#pragma once

#include "myorm/model.hpp"
#include "myorm/database.h"

class DmsRemakeFile : public zel::myorm::Model<DmsRemakeFile> {
  public:
    DmsRemakeFile()
        : Model() {}
    DmsRemakeFile(zel::myorm::Database &db)
        : Model(db()) {}
    DmsRemakeFile(zel::myorm::Connection *conn)
        : Model(conn) {}

    std::string table() const { return "dms_remakefile"; }

    std::string primary_key() const { return "ID"; }
};