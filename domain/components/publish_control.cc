#include "market/domain/components/publish_control.h"
#include <memory>
#include "common/extern/log/log.h"
#include "common/self/utils.h"
#include "market/domain/components/fd_manage.h"

PublishControl::PublishControl(void) {
  InitDatabase();
  PrepareSqlSentence();
  RestoreFromSqlite3();
}

PublishControl::~PublishControl() {
  sqlite3_finalize(insert_control_);
  sqlite3_finalize(delete_control_);
}

void PublishControl::PrepareSqlSentence() {
  const char *sql = "insert into publish_control(ins, exch) VALUES(?, ?);";
  if (sqlite3_prepare_v2(FdManage::GetInstance().GetMarketConn(), sql, strlen(sql), &insert_control_, 0) != SQLITE_OK) {
    ERROR_LOG("prepare sql sentence error.");
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }

  sql = "delete from publish_control where ins = ? and exch = ?;";
  if (sqlite3_prepare_v2(FdManage::GetInstance().GetMarketConn(), sql, strlen(sql), &delete_control_, 0) != SQLITE_OK) {
    ERROR_LOG("prepare sql sentence error.");
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }
}

void PublishControl::RestoreFromSqlite3() {
  int nrow = 0, ncolumn = 0;
  char **result;
  char *error_msg = nullptr;

  const char *sql = "select * from publish_control;";
  if (sqlite3_get_table(FdManage::GetInstance().GetMarketConn(), sql, &result, &nrow, &ncolumn, &error_msg) != SQLITE_OK) {
    ERROR_LOG("sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }
  for (int i = 1; i <= nrow; i++) {
    INFO_LOG("load instrument: %s.", result[i * ncolumn]);
    publish_para_map_[result[i * ncolumn]] = std::make_unique<PublishPara>(result[i * ncolumn + 1], 0);
  }
  sqlite3_free_table(result);
}

void PublishControl::InitDatabase() {
  char *error_msg = nullptr;
  const char *sql = "create table if not exists publish_control(ins TEXT, exch TEXT);";
  if (sqlite3_exec(FdManage::GetInstance().GetMarketConn(), sql, NULL, NULL, &error_msg) != SQLITE_OK) {
    ERROR_LOG("sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }
}

void PublishControl::BuildPublishPara(const std::string &ins, const PublishPara &para) {
  auto iter = publish_para_map_.find(ins);
  if (iter != publish_para_map_.end()) {
    return;
  }
  publish_para_map_[ins] = std::make_unique<PublishPara>(para);
  INFO_LOG("insert ins: %s.", ins.c_str());
  sqlite3_reset(insert_control_);
  sqlite3_bind_text(insert_control_, 1, ins.c_str(), ins.size(), 0);
  sqlite3_bind_text(insert_control_, 2, para.GetExch().c_str(), para.GetExch().size(), 0);
  if (sqlite3_step(insert_control_) != SQLITE_DONE) {
    ERROR_LOG("do sql sentence error.");
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }
}

void PublishControl::ErasePublishPara(const std::string &ins) {
  for (auto publish_iter = publish_para_map_.begin(); publish_iter != publish_para_map_.end();) {
    if (publish_iter->first == ins || ins == "") {
      INFO_LOG("ins: %s, doesn't exist anymore, erase it.", publish_iter->first.c_str());
      sqlite3_reset(delete_control_);
      sqlite3_bind_text(delete_control_, 1, publish_iter->first.c_str(), publish_iter->first.size(), 0);
      sqlite3_bind_text(delete_control_, 2, publish_iter->second->GetExch().c_str(), publish_iter->second->GetExch().size(), 0);
      if (sqlite3_step(delete_control_) != SQLITE_DONE) {
        ERROR_LOG("do sql sentence error.");
        sqlite3_close(FdManage::GetInstance().GetMarketConn());
      }
      publish_iter = publish_para_map_.erase(publish_iter);
    } else {
      publish_iter++;
    }
  }
}

const std::vector<utils::InstrumentID> &PublishControl::GetInstrumentList() {
  instrument_vec_.clear();

  for (auto &item_pc : publish_para_map_) {
    utils::InstrumentID item_ins;
    item_ins.ins = item_pc.first;
    item_ins.exch = item_pc.second->GetExch();
    instrument_vec_.push_back(item_ins);
  }

  return instrument_vec_;
}

const PublishPara *PublishControl::GetPublishPara(const std::string &ins) {
  auto pos = publish_para_map_.find(ins);
  if (pos == publish_para_map_.end()) {
    return nullptr;
  }

  return pos->second.get();
}

const std::unordered_map<std::string, std::unique_ptr<PublishPara>> &PublishControl::GetPublishParaMap() { return publish_para_map_; }