#ifndef MARKET_CONTROL_PARA_H
#define MARKET_CONTROL_PARA_H

#include <set>
#include <string>

#include "common/self/utils.h"

struct ControlPara {
  ControlPara();
  ~ControlPara(){};

  std::set<std::string> GetPridList(void);

  bool InsertControlPara(const std::string &prid);
  bool EraseControlPara(const std::string &prid);

 private:
  std::set<std::string> publish_paras_;
  bool LoadFromJson(void);
  bool WriteToJson(void);
  std::string json_path_ = "";
};

#endif
