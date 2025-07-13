#pragma once

#include "myorm/database.h"
#include "distribution_record.h"

#include <memory>
#include <unordered_map>
#include <xlnt/cell/cell_reference.hpp>
#include <xlnt/workbook/workbook.hpp>
#include <xlnt/xlnt.hpp>

class Tabulation {
  public:
    Tabulation(const std::shared_ptr<zel::myorm::Database> &db);
    ~Tabulation();

    bool distributionRecord(const std::string &order_number, const std::string &data_field);

    void generateDistributionRecords(const std::string &template_file, const std::string &output_file);

  private:
    bool loadTemplate(const std::string &path);

    /// @brief 扫描模板中的位置
    void locateTemplateTags();

    /// @brief 写入业务数据
    void fillTemplateWithData(const std::string &output_file);

    /// @brief 计算单元格位置
    xlnt::cell_reference offset(const xlnt::cell_reference &ref, int row_offset, int col_offset);

    /// @brief 复制行样式
    void copy_row_style(std::uint32_t source_row, std::uint32_t target_row);

  private:
    std::shared_ptr<zel::myorm::Database>                 db_;
    std::shared_ptr<DistributionRecord>                   dr_;
    xlnt::workbook                                        workbook_;
    xlnt::worksheet                                       worksheet_;
    std::unordered_map<std::string, xlnt::cell_reference> cell_refs_;
};