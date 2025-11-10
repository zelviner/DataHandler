#include "dms.h"
#include "model/dms_product_orders.hpp"
#include "model/dms_order_conf.hpp"
#include "model/dms_batch_list.hpp"
#include "model/dms_record_logs.hpp"
#include "model/dms_delete_record.hpp"
#include "model/dms_remake_record.hpp"
#include "model/dms_bind_record.hpp"
#include "model/dms_process_record.hpp"
#include "model/dms_remake.hpp"
#include "model/dms_remake_file.hpp"
#include "model/dms_task_list.hpp"
#include "model/dms_perso_data.hpp"
#include "model/dms_batch_files.hpp"
#include "model/dms_batch_list.hpp"
#include "model/dms_order_conf.hpp"
#include "model/dms_product_orders.hpp"

#include <zel/core.h>

Dms::Dms(const std::shared_ptr<zel::myorm::Database> &db, const std::string &order_no)
    : db_(db) {
    order_info_.order_no = order_no;
    order_info();
}

Dms::~Dms() {}

bool Dms::deleteOrder() {
    // delete about record table and records
    if (!delete_record()) {
        return false;
    }

    // delete about production table and records
    if (!delete_production_data()) {
        return false;
    }

    // delete about order data table and records
    if (!delete_order_data()) {
        return false;
    }

    return true;
}

bool Dms::order_info() {
    DmsProductOrders dms_product_orders(*db_);
    dms_product_orders.where("Code", order_info_.order_no);
    auto dpo_all = dms_product_orders.all();
    if (dpo_all.empty() || dpo_all.size() > 1) {
        log_error("table dms_product_orders has no or more than one record, xh_order_number: %s", order_info_.order_no.c_str());
        return false;
    }

    order_info_.order_id = dpo_all[0]("ID").asInt();

    DmsOrderConf dms_order_conf(*db_);
    dms_order_conf.where("Order", order_info_.order_id);
    auto doc_all = dms_order_conf.all();
    if (doc_all.empty()) {
        log_error("table dms_order_conf has no record, Order: %d", order_info_.order_id);
        return false;
    }

    order_info_.batch_list_id      = doc_all[0]("Batch").asInt();
    order_info_.perso_script_path  = doc_all[0]("Perso").asString();
    order_info_.verify_script_path = doc_all[0]("Verify").asString();
    order_info_.clear_script_path  = doc_all[0]("Clear").asString();

    DmsBatchList dms_batch_list(*db_);
    dms_batch_list.where("ID", order_info_.batch_list_id);
    auto dbl_all = dms_batch_list.all();
    if (dbl_all.empty()) {
        log_error("table dms_batch_list has no record, ID: %d", order_info_.batch_list_id);
        return false;
    }

    auto data_table_name = dbl_all[0]("Uuid").asString();
    String::toLower(data_table_name);
    order_info_.perso_data_table_name = data_table_name;

    log_info("order_no: %s, order_id: %d, batch_list_id: %d, perso_script_path: %s, verify_script_path: %s, clear_script_path: %s, perso_data_table_name: %s",
             order_info_.order_no.c_str(), order_info_.order_id, order_info_.batch_list_id, order_info_.perso_script_path.c_str(),
             order_info_.verify_script_path.c_str(), order_info_.clear_script_path.c_str(), order_info_.perso_data_table_name.c_str());

    return true;
}

bool Dms::delete_record() {
    std::string order_no = order_info_.order_no;
    String::toLower(order_no);

    // delete dms_remake_record table
    DmsRemakeRecord dms_remake_record(*db_, order_no, "ID");
    std::string     sql = "DROP TABLE IF EXISTS `" + dms_remake_record.table() + "`";

    // delete dms_record_logs table
    DmsRecordLogs dms_record_logs(*db_, order_no, "ID");
    sql = "DROP TABLE IF EXISTS `" + dms_record_logs.table() + "`";
    db_->execute(sql);

    // delete dms_deleterecord records
    DmsDeleteRecord(*db_).where("Order", order_info_.order_id).remove();

    // delete dms_bindrecord records
    DmsBindRecord(*db_)
        .where("Perso", order_info_.perso_script_path)
        .where("Verify", order_info_.verify_script_path)
        .where("Clear", order_info_.clear_script_path)
        .remove();

    // delete dms_processrecord records
    DmsProcessRecord(*db_).where("Product", order_info_.order_no).remove();

    return true;
}

bool Dms::delete_production_data() {
    std::string order_no = order_info_.order_no;
    String::toLower(order_no);

    // delete dms_remake table
    DmsRemake   dms_remake(*db_, order_no, "ID");
    std::string sql = "DROP TABLE IF EXISTS `" + dms_remake.table() + "`";
    db_->execute(sql);

    // delete dms_remake_file records
    DmsRemakeFile(*db_).where("Order", order_info_.order_id).remove();

    // delete dms_tasklist records
    DmsTaskList(*db_).where("Order", order_info_.order_id).remove();

    return true;
}

bool Dms::delete_order_data() {
    std::string data_table_name = order_info_.perso_data_table_name;
    String::toLower(data_table_name);

    // delete perso_data table
    DmsPersoData dms_perso_data(*db_, data_table_name, "ID");
    std::string  sql = "DROP TABLE IF EXISTS `" + dms_perso_data.table() + "`";
    db_->execute(sql);

    // delete dms_batch_file records
    DmsBatchFiles(*db_).where("Batch", order_info_.batch_list_id).remove();

    // delete dms_batch_list records
    DmsBatchList(*db_).where("ID", order_info_.batch_list_id).remove();

    // delete dms_orderconf records
    DmsOrderConf(*db_).where("Order", order_info_.order_id).remove();

    // delete dms_productorders records
    DmsProductOrders(*db_).where("Code", order_info_.order_no).remove();

    return true;
}