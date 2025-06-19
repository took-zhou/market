#include "market/domain/components/instrument_info.h"
#include "common/extern/log/log.h"
#include "market/domain/components/fd_manage.h"

InstrumentInfo::InstrumentInfo() {
  InitDatabase();
  PrepareSqlSentence();
  RestoreFromSqlite3();
}

InstrumentInfo::~InstrumentInfo() {
  sqlite3_finalize(insert_instrument_);
  sqlite3_finalize(delete_instrument_);
}

void InstrumentInfo::PrepareSqlSentence() {
  const char *sql =
      "insert into instrument_info(exch, ins, ticksize, max_market_order_volume, min_market_order_volume, max_limit_order_volume, "
      "min_limit_order_volume, tradeuint, is_trading) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);";
  if (sqlite3_prepare_v2(FdManage::GetInstance().GetMarketConn(), sql, strlen(sql), &insert_instrument_, 0) != SQLITE_OK) {
    ERROR_LOG("prepare sql sentence error.");
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }

  sql = "delete from instrument_info;";
  if (sqlite3_prepare_v2(FdManage::GetInstance().GetMarketConn(), sql, strlen(sql), &delete_instrument_, 0) != SQLITE_OK) {
    ERROR_LOG("prepare sql sentence error.");
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }
}

void InstrumentInfo::RestoreFromSqlite3() {}

void InstrumentInfo::InitDatabase() {
  char *error_msg = nullptr;
  const char *sql =
      "create table if not exists instrument_info(exch TEXT, ins TEXT, ticksize REAL, max_market_order_volume INT, min_market_order_volume "
      "INT, max_limit_order_volume INT, min_limit_order_volume INT, tradeuint REAL, is_trading INT);";
  if (sqlite3_exec(FdManage::GetInstance().GetMarketConn(), sql, NULL, NULL, &error_msg) != SQLITE_OK) {
    ERROR_LOG("sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }
}

void InstrumentInfo::BuildInstrumentInfo(const std::string &keyname, const Info &info) { info_map_[keyname] = info; }

void InstrumentInfo::EraseAllInstrumentInfo(void) {
  info_map_.clear();
  INFO_LOG("erase all instrument info.");
}

std::vector<std::string> InstrumentInfo::GetInstrumentList(void) {
  std::vector<std::string> ret_vec;
  for (auto &item : info_map_) {
    ret_vec.push_back(item.first);
  }

  return ret_vec;
}

std::string InstrumentInfo::GetExchange(const std::string &ins) {
  auto iter = info_map_.find(ins);
  if (iter != info_map_.end()) {
    return iter->second.exch;
  } else {
    ERROR_LOG("not find ins: %s", ins.c_str());
    return "";
  }
}

double InstrumentInfo::GetTickSize(const std::string &ins) {
  auto iter = info_map_.find(ins);
  if (iter != info_map_.end()) {
    return iter->second.ticksize;
  } else {
    ERROR_LOG("not find ins: %s", ins.c_str());
    return -1;
  }
}

InstrumentInfo::Info *InstrumentInfo::GetInstrumentInfo(const std::string &ins) {
  auto iter = info_map_.find(ins);
  if (iter != info_map_.end()) {
    return &iter->second;
  } else {
    ERROR_LOG("not find ins: %s", ins.c_str());
    return nullptr;
  }
}

void InstrumentInfo::ShowInstrumentInfo() { INFO_LOG("the size of info map is: %d", static_cast<int>(info_map_.size())); }

void InstrumentInfo::UpdateInstrumentInfo() {
  sqlite3_reset(delete_instrument_);
  if (sqlite3_step(delete_instrument_) != SQLITE_DONE) {
    ERROR_LOG("do sql sentence error.");
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }

  for (auto &item : info_map_) {
    sqlite3_reset(insert_instrument_);
    sqlite3_bind_text(insert_instrument_, 1, item.second.exch.c_str(), item.second.exch.size(), 0);
    sqlite3_bind_text(insert_instrument_, 2, item.first.c_str(), item.first.size(), 0);
    sqlite3_bind_double(insert_instrument_, 3, item.second.ticksize);
    sqlite3_bind_int(insert_instrument_, 4, item.second.max_market_order_volume);
    sqlite3_bind_int(insert_instrument_, 5, item.second.min_market_order_volume);
    sqlite3_bind_int(insert_instrument_, 6, item.second.max_limit_order_volume);
    sqlite3_bind_int(insert_instrument_, 7, item.second.min_limit_order_volume);
    sqlite3_bind_double(insert_instrument_, 8, item.second.tradeuint);
    sqlite3_bind_int(insert_instrument_, 9, item.second.is_trading);
    if (sqlite3_step(insert_instrument_) != SQLITE_DONE) {
      ERROR_LOG("do sql sentence error.");
      sqlite3_close(FdManage::GetInstance().GetMarketConn());
    }
  }
  INFO_LOG("update instrument info ok");
}
