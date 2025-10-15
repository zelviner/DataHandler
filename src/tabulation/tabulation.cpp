#include "tabulation.h"
#include "distribution_record.h"
#include "model/xh_order_list.hpp"
#include "model/xh_datatool_record.hpp"
#include "model/dms_product_orders.hpp"
#include "model/dms_order_conf.hpp"
#include "model/dms_batch_list.hpp"
#include "model/dms_batch_files.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>
#include <xlnt/cell/cell_reference.hpp>
#include <xlnt/workbook/workbook.hpp>
#include <zel/utility/ini_file.h>
#include <zel/utility/logger.h>
#include <fmt/format.h>
#include <zel/utility/string.h>
#include <zel/file_system/file.h>

Tabulation::Tabulation(const std::shared_ptr<zel::myorm::Database> &finance_db, const std::shared_ptr<zel::myorm::Database> &telecom_db,
                       const zel::utility::Ini &ini)
    : finance_db_(finance_db)
    , telecom_db_(telecom_db)
    , ini_(ini)
    , dr_(std::make_shared<DistributionRecord>())
    , cell_refs_(std::unordered_map<std::string, xlnt::cell_reference>()) {
    key_map_["order_no"]       = ini_["template"]["order_no"].asString();
    key_map_["order_quantity"] = ini_["template"]["order_quantity"].asString();
    key_map_["data"]           = ini_["template"]["data"].asString();
}

Tabulation::~Tabulation() {}

std::vector<std::string> Tabulation::financeOrderList() {
    std::vector<std::string> order_list;

    XhOrderList xh_order_list(*finance_db_);
    auto        ol_all = xh_order_list.all();
    for (auto one : ol_all) {
        order_list.push_back(one("xh_order_number").asString());
    }

    return order_list;
}

std::vector<std::string> Tabulation::telecomOrderList() {
    std::vector<std::string> order_list;

    DmsProductOrders dms_product_orders(*telecom_db_);
    auto             ol_all = dms_product_orders.all();
    for (auto one : ol_all) {
        order_list.push_back(one("Code").asString());
    }

    return order_list;
}

bool Tabulation::financeRecords(const std::string &order_number, const std::string &data_field) {
    dr_->header.order_no = order_number;

    XhOrderList xh_order_list(*finance_db_);
    xh_order_list.where("xh_order_number", order_number);
    auto ol_all = xh_order_list.all();
    if (ol_all.empty() || ol_all.size() > 1) {
        log_error("table xh_order_list has no or more than one record, xh_order_number: %s", order_number.c_str());
        return false;
    }

    auto rid = ol_all[0]("RID").asInt();

    XhDatatoolRecord xh_datatool_record(*finance_db_);
    xh_datatool_record.where("RID", rid);
    auto dr_all = xh_datatool_record.all();
    if (dr_all.empty()) {
        log_error("table xh_datatool_record has no record, RID: %d", rid);
        return false;
    }

    for (auto one : dr_all) {
        DistributionRecordData data;
        data.filename = one("xh_order_filename").asString();
        if (data.filename.find(".gpg") == std::string::npos || data.filename.find(".pgp") == std::string::npos) {
            data.filename = data.filename.substr(0, data.filename.length() - 4);
        }

        // 查询文件数量
        String::toLower(data.filename);
        std::string sql    = fmt::format("SELECT COUNT(*), MIN(ID), MAX(ID) FROM `{}`", data.filename);
        auto        result = finance_db_->query(sql);
        data.quantity      = result[0].find("COUNT(*)")->second.asInt();
        std::string min_id = result[0].find("MIN(ID)")->second.asString();
        std::string max_id = result[0].find("MAX(ID)")->second.asString();

        sql    = fmt::format("SELECT {} FROM `{}` WHERE ID='{}'", data_field, data.filename, min_id);
        result = finance_db_->query(sql);
        if (result.empty()) {
            log_error("query failed, sql: %s", sql.c_str());
            break;
        }
        data.start_iccid = result[0].find(data_field)->second.asString();

        sql            = fmt::format("SELECT {} FROM `{}` WHERE ID='{}'", data_field, data.filename, max_id);
        result         = finance_db_->query(sql);
        data.end_iccid = result[0].find(data_field)->second.asString();

        dr_->datas.push_back(data);
    }

    dr_->header.order_quantity = dr_->datas[0].quantity * dr_->datas.size();

    return true;
}

bool Tabulation::telecomRecords(const std::string &order_number, const std::string &data_field) {
    dr_->header.order_no = order_number;

    DmsProductOrders dms_product_orders(*telecom_db_);
    dms_product_orders.where("Code", order_number);
    auto dpo_all = dms_product_orders.all();
    if (dpo_all.empty() || dpo_all.size() > 1) {
        log_error("table dms_product_orders has no or more than one record, xh_order_number: %s", order_number.c_str());
        return false;
    }

    auto order_id = dpo_all[0]("ID").asInt();

    DmsOrderConf dms_order_conf(*telecom_db_);
    dms_order_conf.where("Order", order_id);
    auto doc_all = dms_order_conf.all();
    if (doc_all.empty()) {
        log_error("table dms_order_conf has no record, Order: %d", order_id);
        return false;
    }

    auto batch_list_id = doc_all[0]("Batch").asInt();

    DmsBatchList dms_batch_list(*telecom_db_);
    dms_batch_list.where("ID", batch_list_id);
    auto dbl_all = dms_batch_list.all();
    if (dbl_all.empty()) {
        log_error("table dms_batch_list has no record, ID: %d", batch_list_id);
        return false;
    }

    auto data_table = dbl_all[0]("Uuid").asString();
    String::toLower(data_table);

    // 创建索引，加速查询
    std::string sql = fmt::format("CREATE INDEX idx_file_id ON `{}` (File, ID)", data_table);
    telecom_db_->execute(sql);

    DmsBatchFiles dms_batch_files(*telecom_db_);
    dms_batch_files.where("Batch", batch_list_id);
    auto dbf_all = dms_batch_files.all();
    if (dbf_all.empty()) {
        log_error("table dms_batch_files has no record, Batch: %d", batch_list_id);
        return false;
    }

    for (auto one : dbf_all) {
        DistributionRecordData data;
        data.filename = one("Filename").asString();
        if (data.filename.find("Remake") != std::string::npos) {
            continue;
        }

        // 去掉文件名后缀
        size_t index = data.filename.find(".");
        if (index != std::string::npos) {
            data.filename = data.filename.substr(0, index);
        }

        // 查询文件数量
        data.quantity = one("Quantity").asInt();

        String::toLower(data.filename);
        std::string sql      = fmt::format("SELECT MIN(ID), MAX(ID) FROM `{}` WHERE File = {}", data_table, one("ID").asInt());
        auto        result   = telecom_db_->query(sql);
        int         start_id = result[0].find("MIN(ID)")->second.asInt();
        int         end_id   = result[0].find("MAX(ID)")->second.asInt();

        sql              = fmt::format("SELECT {} FROM `{}` WHERE ID = {} OR ID = {}", data_field, data_table, start_id, end_id);
        result           = telecom_db_->query(sql);
        data.start_iccid = result[0].find(data_field)->second.asString();
        data.end_iccid   = result[1].find(data_field)->second.asString();
        exchange_iccid(data.start_iccid);
        exchange_iccid(data.end_iccid);

        dr_->datas.push_back(data);
        dr_->header.order_quantity += data.quantity;
    }

    // 删除索引
    sql = fmt::format("DROP INDEX idx_file_id ON `{}`", data_table);
    telecom_db_->execute(sql);

    return true;
}

void Tabulation::generatingFinanceRecords(const std::string &template_file, const std::string &output_file) {
    zel::file_system::File file(template_file);
    if (!file.exists()) {
        log_error("template file not exists: %s", template_file.c_str());
        return;
    }

    if (!load_template(template_file)) {
        log_error("load template failed: %s", template_file.c_str());
        return;
    }

    locate_template_tags();

    if (!file.copy(output_file)) {
        log_error("copy file failed: %s -> %s", template_file.c_str(), output_file.c_str());
        return;
    }

    load_template(output_file);
    fill_template_with_data(output_file);
}

void Tabulation::generatingTelecomRecords(const std::string &template_file, const std::string &output_file) {
    zel::file_system::File file(template_file);
    if (!file.exists()) {
        log_error("template file not exists: %s", template_file.c_str());
        return;
    }

    if (!load_template(template_file)) {
        log_error("load template failed: %s", template_file.c_str());
        return;
    }

    locate_template_tags();

    if (!file.copy(output_file)) {
        log_error("copy file failed: %s -> %s", template_file.c_str(), output_file.c_str());
        return;
    }

    // 注意：output_file 中的数据还没写入，所以要重新 load
    load_template(output_file);
    fill_template_with_data(output_file);
}

bool Tabulation::load_template(const std::string &path) {
    workbook_.load(path);
    worksheet_ = workbook_.active_sheet();
    return true;
}

void Tabulation::locate_template_tags() {
    int rowIndex = 0;
    for (const auto &row : worksheet_.rows()) {
        rowIndex++;
        if (rowIndex <= 2) continue; // 跳过前两行

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

void Tabulation::fill_template_with_data(const std::string &output_file) {
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

void Tabulation::exchange_iccid(std::string &iccid) {
    for (size_t i = 0; i + 1 < iccid.size(); i += 2) {
        std::swap(iccid[i], iccid[i + 1]);
    }
}