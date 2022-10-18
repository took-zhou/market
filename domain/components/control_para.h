#ifndef MARKET_CONTROL_PARA_H
#define MARKET_CONTROL_PARA_H

#include <string>
#include <unordered_map>
#include <vector>

#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"

struct PublishControl {
  std::string prid;
  std::string exch;
  std::string begin;
  std::string end;
  uint32_t speed;
  strategy_market::TickStartStopIndication_MessageType indication = strategy_market::TickStartStopIndication_MessageType_reserve;
  uint32_t interval = 0;
  bool directforward = false;
  std::string source = "rawtick";
  mutable uint32_t heartbeat = 0;
};

struct ControlPara {
  ControlPara();
  ~ControlPara(){};

  std::vector<utils::InstrumtntID> GetInstrumentList(const std::string &prid = "");
  std::vector<std::string> GetPridList(void);
  int GetInstrumentSubscribedCount(const utils::InstrumtntID &instruemtn_id);
  void SetStartStopIndication(const std::string keyname, strategy_market::TickStartStopIndication_MessageType indication);

  void BuildControlPara(const std::string &keyname, const PublishControl &para);
  void EraseControlPara(const std::string &keyname, const std::string &ins = "");

  std::unordered_map<std::string, std::vector<PublishControl>> publish_ctrl_map;

 private:
  bool LoadFromJson(void);
  bool WriteToJson(void);
  std::string json_path_ = "";
};

#endif
