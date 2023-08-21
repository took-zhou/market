#include <fstream>
#include <iomanip>

#include "market/domain/components/backtest_control.h"

#include "common/extern/json/fifo_map.hpp"
#include "common/extern/json/json.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"
#include "market/domain/components/fd_manage.h"

BacktestControl::BacktestControl(void) {
  init_database_flag_ = false;
  InitDatabase();
  PrepareSqlSentence();
  RestoreFromSqlite3();
}

BacktestControl::~BacktestControl() {
  sqlite3_finalize(update_control_);
  sqlite3_finalize(update_indication_);
}

void BacktestControl::PrepareSqlSentence() {
  const char *sql = "update backtest_control set begin = ?, end = ?, speed = ?, source = ?;";
  if (sqlite3_prepare_v2(FdManage::GetInstance().market_conn, sql, strlen(sql), &update_control_, 0) != SQLITE_OK) {
    ERROR_LOG("prepare sql sentence error.");
    sqlite3_close(FdManage::GetInstance().market_conn);
  }

  sql = "update backtest_control set indication = ?;";
  if (sqlite3_prepare_v2(FdManage::GetInstance().market_conn, sql, strlen(sql), &update_indication_, 0) != SQLITE_OK) {
    ERROR_LOG("prepare sql sentence error.");
    sqlite3_close(FdManage::GetInstance().market_conn);
  }
}

void BacktestControl::RestoreFromSqlite3() {
  int nrow = 0, ncolumn = 0;
  char **result;
  char *error_msg = nullptr;

  const char *sql = "select * from backtest_control;";
  if (sqlite3_get_table(FdManage::GetInstance().market_conn, sql, &result, &nrow, &ncolumn, &error_msg) != SQLITE_OK) {
    ERROR_LOG("Sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(FdManage::GetInstance().market_conn);
  }
  if (nrow == 1) {
    backtest_para_.begin = result[ncolumn];
    backtest_para_.end = result[ncolumn + 1];
    backtest_para_.now = result[ncolumn + 2];
    backtest_para_.speed = stoi(result[ncolumn + 3]);
    backtest_para_.source = static_cast<ctpview_market::BackTestControl_Source>(stoi(result[ncolumn + 4]));
    backtest_para_.indication = static_cast<ctpview_market::TickStartStopIndication_MessageType>(stoi(result[ncolumn + 5]));
  }
  sqlite3_free_table(result);
}

void BacktestControl::InitDatabase() {
  if (init_database_flag_ == false) {
    char *error_msg = nullptr;
    const char *sql = "create table if not exists backtest_control(begin TEXT, end TEXT, now TEXT, speed INT, source INT, indication INT);";
    if (sqlite3_exec(FdManage::GetInstance().market_conn, sql, NULL, NULL, &error_msg) != SQLITE_OK) {
      ERROR_LOG("Sql error %s.", error_msg);
      sqlite3_free(error_msg);
      sqlite3_close(FdManage::GetInstance().market_conn);
    }

    sql =
        "insert into backtest_control(begin, end, now, speed, source, indication) select '2015-01-01 09:00:00', '2020-12-31 15:00:00', '', "
        "1, 0, 0 where not exists (select * from backtest_control);";
    if (sqlite3_exec(FdManage::GetInstance().market_conn, sql, NULL, NULL, &error_msg) != SQLITE_OK) {
      ERROR_LOG("Sql error %s.", error_msg);
      sqlite3_free(error_msg);
      sqlite3_close(FdManage::GetInstance().market_conn);
    }
    init_database_flag_ = true;
  }
}

void BacktestControl::BuildControlPara(const BacktestPara &para) {
  backtest_para_.begin = para.begin;
  backtest_para_.end = para.end;
  backtest_para_.speed = para.speed;
  backtest_para_.source = para.source;
  sqlite3_reset(update_control_);
  sqlite3_bind_text(update_control_, 1, para.begin.c_str(), para.begin.size(), 0);
  sqlite3_bind_text(update_control_, 2, para.end.c_str(), para.end.size(), 0);
  sqlite3_bind_int(update_control_, 3, para.speed);
  sqlite3_bind_int(update_control_, 4, para.source);
  if (sqlite3_step(update_control_) != SQLITE_DONE) {
    ERROR_LOG("do sql sentence error.");
    sqlite3_close(FdManage::GetInstance().market_conn);
  }
}

void BacktestControl::SetStartStopIndication(ctpview_market::TickStartStopIndication_MessageType indication) {
  backtest_para_.indication = indication;
  sqlite3_reset(update_indication_);
  sqlite3_bind_int(update_indication_, 1, indication);
  if (sqlite3_step(update_indication_) != SQLITE_DONE) {
    ERROR_LOG("do sql sentence error.");
    sqlite3_close(FdManage::GetInstance().market_conn);
  }
}
