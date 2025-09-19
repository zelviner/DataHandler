#pragma once

#include "myorm/database.h"
#include "tabulation/tabulation.h"

#include <qobjectdefs.h>
#include <zel/utility/ini_file.h>
#include <qcoreapplication>
#include <qthread>

class GeneratingRecords : public QThread {
    Q_OBJECT

  public:
    GeneratingRecords(bool telecom_mode, const std::shared_ptr<zel::myorm::Database> &finance_db, const std::shared_ptr<zel::myorm::Database> &telecom_db,
                      const zel::utility::IniFile &ini, const std::string &order_number, const std::string &data_field, const std::string &template_path,
                      const std::string &generate_path)
        : telecom_mode_(telecom_mode)
        , finance_db_(finance_db)
        , telecom_db_(telecom_db)
        , ini_(ini)
        , order_number_(order_number)
        , data_field_(data_field)
        , template_path_(template_path)
        , generate_path_(generate_path) {}
    ~GeneratingRecords() {}

    void run() override {
        Tabulation tabulation(finance_db_, telecom_db_, ini_);
        if (telecom_mode_) {
            if (!tabulation.telecomRecords(order_number_, data_field_)) {
                emit failure();
                return;
            }

            tabulation.generatingTelecomRecords(template_path_, generate_path_);
        } else {
            if (!tabulation.financeRecords(order_number_, data_field_)) {
                emit failure();
                return;
            }

            tabulation.generatingFinanceRecords(template_path_, generate_path_);
        }

        emit success();
    }

  signals:
    void success();
    void failure();

  private:
    bool                                  telecom_mode_;
    std::shared_ptr<zel::myorm::Database> finance_db_;
    std::shared_ptr<zel::myorm::Database> telecom_db_;
    zel::utility::IniFile                 ini_;
    std::string                           order_number_;
    std::string                           data_field_;
    std::string                           template_path_;
    std::string                           generate_path_;
};