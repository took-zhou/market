#ifndef MARKET_PUBLISH_CONTROL_H
#define MARKET_PUBLISH_CONTROL_H

#include <string>
#include <unordered_map>
#include <vector>

#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"

struct PublishPara {
  /*realtime*/
  std::string exch = "";
  strategy_market::TickSubscribeReq_Source source = strategy_market::TickSubscribeReq_Source_rawtick;
  mutable uint32_t heartbeat = 0;
};

struct PublishControl {
  PublishControl();
  ~PublishControl(){};

  void BuildPublishPara(const std::string &ins, const PublishPara &para);
  void ErasePublishPara(const std::string &ins = "");

  std::vector<utils::InstrumtntID> GetInstrumentList();
  std::unordered_map<std::string, PublishPara> publish_para_map;

 private:
  bool LoadFromJson(void);
  bool WriteToJson(void);
  std::string json_path_ = "";
};

#endif
