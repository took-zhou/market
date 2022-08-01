#ifndef CONTROL_PARA_H
#define CONTROL_PARA_H

#include <string>
#include <unordered_map>
#include <vector>

#include "common/self/basetype.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"

struct publishControl {
  std::string identify;
  std::string exch;
  U32 ticksize;
  strategy_market::TickStartStopIndication_MessageType indication = strategy_market::TickStartStopIndication_MessageType_reserve;
  U32 interval = 0;
  bool directforward = false;
  std::string source = "rawtick";
  mutable U32 heartbeat = 0;
};

struct controlPara {
  controlPara();
  ~controlPara(){};

  bool load_from_json(void);

  std::vector<utils::InstrumtntID> getInstrumentList(void);
  std::vector<std::string> getIdentifyList(void);

  void buildControlPara(const std::string &keyname, const publishControl &para);
  void eraseControlPara(const std::string &keyname);

  void setStartStopIndication(const std::string keyname, strategy_market::TickStartStopIndication_MessageType _indication);

  std::unordered_map<std::string, std::vector<publishControl>> publishCtrlMap;

 private:
  bool write_to_json(void);
  std::string json_path = "";
};

#endif
