#include "tabulation.h"
#include "distribution_record.h"
#include "model/xh_order_list.hpp"
#include "model/xh_dataload_record.hpp"

#include <memory>
#include <vector>
#include <xlnt/cell/cell_reference.hpp>
#include <xlnt/workbook/workbook.hpp>
#include <zel/utility/ini_file.h>
#include <zel/utility/logger.h>
#include <fmt/format.h>
#include <zel/utility/string.h>
#include <zel/filesystem/file.h>

Tabulation::Tabulation(const std::shared_ptr<zel::myorm::Database> &db, const zel::utility::IniFile &ini)
    : db_(db)
    , ini_(ini)
    , dr_(std::make_shared<DistributionRecord>())
    , cell_refs_(std::unordered_map<std::string, xlnt::cell_reference>()) {
    key_map_["order_no"]       = ini_["template"]["order_no"].asString();
    key_map_["order_quantity"] = ini_["template"]["order_quantity"].asString();
    key_map_["data"]           = ini_["template"]["data"].asString();
}

Tabulation::~Tabulation() {}

std::vector<std::string> Tabulation::orderList() {
    std::vector<std::string> order_list;

    XhOrderList xh_order_list(*db_);
    auto        ol_all = xh_order_list.all();
    for (auto one : ol_all) {
        order_list.push_back(one("xh_order_number").asString());
    }

    return order_list;
}

bool Tabulation::distributionRecord(const std::string &order_number, const std::string &data_field) {
    dr_->header.order_no = order_number;

    XhOrderList xh_order_list(*db_);
    xh_order_list.where("xh_order_number", order_number);
    auto ol_all = xh_order_list.all();
    if (ol_all.empty() || ol_all.size() > 1) {
        log_error("xh_order_list.all() failed, order_number: %s", order_number.c_str());
        return false;
    }

    auto xh_order_number = ol_all[0]("xh_order_number").asInt();

    XhDataloadRecord xh_dataload_record(*db_);
    xh_dataload_record.where("ProjectID", xh_order_number);
    auto dr_all = xh_dataload_record.all();
    if (dr_all.empty()) {
        log_error("xh_dataload_record.all() failed, xh_order_number: %d", xh_order_number);
        return false;
    }

    for (auto one : dr_all) {
        DistributionRecordData data;
        data.filename = one("DataFileName").asString();

        // 查询文件数量
        String::toLower(data.filename);
        std::string sql    = fmt::format("SELECT COUNT(*), MIN(ID), MAX(ID) FROM `{}`", data.filename);
        auto        result = db_->query(sql);
        data.quantity      = result[0].find("COUNT(*)")->second.asInt();
        std::string min_id = result[0].find("MIN(ID)")->second.asString();
        std::string max_id = result[0].find("MAX(ID)")->second.asString();

        sql    = fmt::format("SELECT {} FROM `{}` WHERE ID='{}'", data_field, data.filename, min_id);
        result = db_->query(sql);
        if (result.empty()) {
            log_error("query failed, sql: %s", sql.c_str());
            break;
        }
        data.start_iccid = result[0].find(data_field)->second.asString();

        sql            = fmt::format("SELECT {} FROM `{}` WHERE ID='{}'", data_field, data.filename, max_id);
        result         = db_->query(sql);
        data.end_iccid = result[0].find(data_field)->second.asString();

        dr_->datas.push_back(data);
    }

    dr_->header.order_quantity = dr_->datas[0].quantity * dr_->datas.size();

    return true;
}

void Tabulation::generateDistributionRecords(const std::string &template_file, const std::string &output_file) {
    zel::filesystem::File file(template_file);
    if (!file.exists()) {
        log_error("template file not exists: %s", template_file.c_str());
        return;
    }

    if (!loadTemplate(template_file)) {
        log_error("load template failed: %s", template_file.c_str());
        return;
    }

    locateTemplateTags();

    if (!file.copy(output_file)) {
        log_error("copy file failed: %s -> %s", template_file.c_str(), output_file.c_str());
        return;
    }

    // 注意：output_file 中的数据还没写入，所以要重新 load
    loadTemplate(output_file);
    fillTemplateWithData(output_file);
}

bool Tabulation::loadTemplate(const std::string &path) {
    workbook_.load(path);
    worksheet_ = workbook_.active_sheet();
    return true;
}

void Tabulation::locateTemplateTags() {

    for (const auto &row : worksheet_.rows()) {
        for (const auto &cell : row) {
            if (!cell.has_value()) continue;
            std::string val = cell.to_string();

            for (const auto &pair : key_map_) {
                if (val.find(pair.second) != std::string::npos) {
                    cell_refs_[pair.first] = cell.reference();
                }
            }
        }
    }
}

void Tabulation::fillTemplateWithData(const std::string &output_file) {
    worksheet_.cell(cell_refs_["order_no"]).value(key_map_["order_no"] + dr_->header.order_no);
    worksheet_.cell(cell_refs_["order_quantity"]).value(key_map_["order_quantity"] + std::to_string(dr_->header.order_quantity));

    auto data_ref = cell_refs_["data"];
    for (size_t i = 0; i < dr_->datas.size(); ++i) {
        const auto &data       = dr_->datas[i];
        int         row_offset = static_cast<int>(i) + 1;

        worksheet_.cell(offset(data_ref, row_offset, 0)).value(data.filename);
        worksheet_.cell(offset(data_ref, row_offset, 1)).value(data.quantity);
        worksheet_.cell(offset(data_ref, row_offset, 2)).value(data.start_iccid);
        worksheet_.cell(offset(data_ref, row_offset, 3)).value(data.end_iccid);
    }

    workbook_.save(output_file);
}

xlnt::cell_reference Tabulation::offset(const xlnt::cell_reference &ref, int row_offset, int col_offset) {
    return xlnt::cell_reference(xlnt::column_t(ref.column().index + col_offset), ref.row() + row_offset);
}

void Tabulation::copy_row_style(std::uint32_t source_row, std::uint32_t target_row) {}