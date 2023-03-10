#ifndef MARKET_BACKTEST_CONTROL_H
#define MARKET_BACKTEST_CONTROL_H

#include <string>
#include <unordered_map>
#include <vector>

#include "common/self/protobuf/ctpview-market.pb.h"
#include "common/self/utils.h"

struct BacktestPara {
  std::string begin = "";
  std::string end = "";
  std::string now = "";
  uint32_t speed = 0;
  ctpview_market::TickStartStopIndication_MessageType indication = ctpview_market::TickStartStopIndication_MessageType_reserve;
};

struct BacktestControl {
  BacktestControl();
  ~BacktestControl(){};

  void SetStartStopIndication(ctpview_market::TickStartStopIndication_MessageType indication);
  void BuildControlPara(const BacktestPara &para);

 private:
  bool LoadFromJson(void);
  bool WriteToJson(void);
  BacktestPara backtest_para_;
  std::string json_path_ = "";
};

#endif
