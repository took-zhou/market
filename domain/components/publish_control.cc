#include "market/domain/components/publish_control.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"
#include "market/domain/components/fd_manage.h"

PublishControl::PublishControl(void) {
  init_database_flag = false;
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
  if (sqlite3_prepare_v2(FdManage::GetInstance().market_conn, sql, strlen(sql), &insert_control_, 0) != SQLITE_OK) {
    ERROR_LOG("prepare sql sentence error.");
    sqlite3_close(FdManage::GetInstance().market_conn);
  }

  sql = "delete from publish_control where ins = ? and exch = ?;";
  if (sqlite3_prepare_v2(FdManage::GetInstance().market_conn, sql, strlen(sql), &delete_control_, 0) != SQLITE_OK) {
    ERROR_LOG("prepare sql sentence error.");
    sqlite3_close(FdManage::GetInstance().market_conn);
  }
}

void PublishControl::RestoreFromSqlite3() {
  int nrow = 0, ncolumn = 0;
  char **result;
  char *error_msg = nullptr;

  const char *sql = "select * from publish_control;";
  if (sqlite3_get_table(FdManage::GetInstance().market_conn, sql, &result, &nrow, &ncolumn, &error_msg) != SQLITE_OK) {
    ERROR_LOG("Sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(FdManage::GetInstance().market_conn);
  }
  for (int i = 1; i <= nrow; i++) {
    PublishPara temp_control;
    temp_control.exch = result[i * ncolumn + 1];
    INFO_LOG("load instrument: %s.", result[i * ncolumn]);
    publish_para_map.insert(make_pair(result[i * ncolumn], temp_control));
  }
  sqlite3_free_table(result);
}

void PublishControl::InitDatabase() {
  if (init_database_flag == false) {
    char *error_msg = nullptr;
    const char *sql = "create table if not exists publish_control(ins TEXT, exch TEXT);";
    if (sqlite3_exec(FdManage::GetInstance().market_conn, sql, NULL, NULL, &error_msg) != SQLITE_OK) {
      ERROR_LOG("Sql error %s.", error_msg);
      sqlite3_free(error_msg);
      sqlite3_close(FdManage::GetInstance().market_conn);
    }
    init_database_flag = true;
  }
}

void PublishControl::BuildPublishPara(const std::string &ins, const PublishPara &para) {
  auto iter = publish_para_map.find(ins);
  if (iter == publish_para_map.end()) {
    publish_para_map[ins] = para;
    INFO_LOG("insert ins: %s.", ins.c_str());
    sqlite3_reset(insert_control_);
    sqlite3_bind_text(insert_control_, 1, ins.c_str(), ins.size(), 0);
    sqlite3_bind_text(insert_control_, 2, para.exch.c_str(), para.exch.size(), 0);
    if (sqlite3_step(insert_control_) != SQLITE_DONE) {
      ERROR_LOG("do sql sentence error.");
      sqlite3_close(FdManage::GetInstance().market_conn);
    }
  }
}

void PublishControl::ErasePublishPara(const std::string &ins) {
  for (auto publish_iter = publish_para_map.begin(); publish_iter != publish_para_map.end();) {
    if (publish_iter->first == ins || ins == "") {
      INFO_LOG("ins: %s, doesn't exist anymore, erase it.", publish_iter->first.c_str());
      sqlite3_reset(delete_control_);
      sqlite3_bind_text(delete_control_, 1, publish_iter->first.c_str(), publish_iter->first.size(), 0);
      sqlite3_bind_text(delete_control_, 2, publish_iter->second.exch.c_str(), publish_iter->second.exch.size(), 0);
      if (sqlite3_step(delete_control_) != SQLITE_DONE) {
        ERROR_LOG("do sql sentence error.");
        sqlite3_close(FdManage::GetInstance().market_conn);
      }
      publish_iter = publish_para_map.erase(publish_iter);
    } else {
      publish_iter++;
    }
  }
}

std::vector<utils::InstrumtntID> PublishControl::GetInstrumentList() {
  std::vector<utils::InstrumtntID> instrument_vec;
  instrument_vec.clear();

  for (auto &item_pc : publish_para_map) {
    utils::InstrumtntID item_ins;
    item_ins.ins = item_pc.first;
    item_ins.exch = item_pc.second.exch;
    instrument_vec.push_back(item_ins);
  }

  return instrument_vec;
}
