#pragma once

#include "distribution_record.h"

#include <memory>
#include <qlist.h>
#include <unordered_map>
#include <xlnt/cell/cell_reference.hpp>
#include <xlnt/workbook/workbook.hpp>
#include <xlnt/xlnt.hpp>
#include <zel/core.h>
#include <zel/myorm.h>

class Tabulation {
  public:
    Tabulation(const std::shared_ptr<zel::myorm::Database> &finance_db, const std::shared_ptr<zel::myorm::Database> &telecom_db, const zel::utility::Ini &ini);
    ~Tabulation();

    std::vector<std::string> financeOrderList();
    std::vector<std::string> telecomOrderList();

    bool financeRecords(const std::string &order_number, const std::string &data_field);
    bool telecomRecords(const std::string &order_number, const std::string &data_field);

    void generatingFinanceRecords(const std::string &template_file, const std::string &output_file);
    void generatingTelecomRecords(const std::string &template_file, const std::string &output_file);

  private:
    bool load_template(const std::string &path);

    /// @brief 扫描模板中的位置
    void locate_template_tags();

    /// @brief 写入业务数据
    void fill_template_with_data(const std::string &output_file);

    /// @brief 计算单元格位置
    xlnt::cell_reference offset(const xlnt::cell_reference &ref, int row_offset, int col_offset);

    /// @brief 两两交换 iccid
    void exchange_iccid(std::string &iccid);

    static bool windows_filename_less(const std::string& a, const std::string& b);

  private:
    std::shared_ptr<zel::myorm::Database>                 finance_db_;
    std::shared_ptr<zel::myorm::Database>                 telecom_db_;
    zel::utility::Ini                                     ini_;
    std::shared_ptr<DistributionRecord>                   dr_;
    xlnt::workbook                                        workbook_;
    xlnt::worksheet                                       worksheet_;
    std::unordered_map<std::string, xlnt::cell_reference> cell_refs_;
    std::unordered_map<std::string, std::string>          key_map_;
};