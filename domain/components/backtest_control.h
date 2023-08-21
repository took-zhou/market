#ifndef MARKET_BACKTEST_CONTROL_H
#define MARKET_BACKTEST_CONTROL_H

#include <string>
#include <unordered_map>
#include <vector>

#include "common/extern/sqlite3/sqlite3.h"
#include "common/self/protobuf/ctpview-market.pb.h"
#include "common/self/utils.h"

struct BacktestPara {
  std::string begin = "";
  std::string end = "";
  std::string now = "";
  uint32_t speed = 0;
  ctpview_market::BackTestControl_Source source = ctpview_market::BackTestControl_Source_rawtick;
  ctpview_market::TickStartStopIndication_MessageType indication = ctpview_market::TickStartStopIndication_MessageType_reserve;
};

struct BacktestControl {
  BacktestControl();
  ~BacktestControl();

  void SetStartStopIndication(ctpview_market::TickStartStopIndication_MessageType indication);
  void BuildControlPara(const BacktestPara &para);

 private:
  void PrepareSqlSentence();
  void RestoreFromSqlite3();
  void InitDatabase();
  sqlite3_stmt *update_control_ = nullptr;
  sqlite3_stmt *update_indication_ = nullptr;
  BacktestPara backtest_para_;
  bool init_database_flag_ = false;
};

#endif
