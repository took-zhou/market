#ifndef CONTROL_PARA_H
#define CONTROL_PARA_H

#include <string>
#include <unordered_map>
#include <vector>

#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"

struct PublishControl {
  std::string prid;
  std::string exch;
  float ticksize;
  strategy_market::TickStartStopIndication_MessageType indication = strategy_market::TickStartStopIndication_MessageType_reserve;
  uint32_t interval = 0;
  bool directforward = false;
  std::string source = "rawtick";
  mutable uint32_t heartbeat = 0;
};

struct ControlPara {
  ControlPara();
  ~ControlPara(){};

  std::vector<utils::InstrumtntID> get_instrument_list(void);
  std::vector<std::string> get_prid_list(void);
  void set_start_stop_indication(const std::string keyname, strategy_market::TickStartStopIndication_MessageType _indication);

  void BuildControlPara(const std::string &keyname, const PublishControl &para);
  void EraseControlPara(const std::string &keyname);

  std::unordered_map<std::string, std::vector<PublishControl>> publish_ctrl_map;

 private:
  bool LoadFromJson(void);
  bool WriteToJson(void);
  std::string json_path = "";
};

#endif
