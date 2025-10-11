#pragma once

#include "dms_order_info.h"

#include <memory>
#include <myorm/database.h>

class Dms {

  public:
    Dms(const std::shared_ptr<zel::myorm::Database> &db, const std::string &order_no);
    ~Dms();

    bool deleteOrder();

  private:
    bool orderInfo();

    bool deleteRecord();

    bool deleteProductionData();

    bool deleteOrderData();

  private:
    std::shared_ptr<zel::myorm::Database> db_;
    DmsOrderInfo                          order_info_;
};