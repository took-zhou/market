#ifndef __MARKETSENDAPI__
#define __MARKETSENDAPI__
#include <vector>
#include "common/self/utils.h"

struct SendApi {
 public:
  virtual bool ReqUserLogin() = 0;
  virtual bool ReqUserLogout() = 0;
  virtual bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec) = 0;
  virtual bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec) = 0;
  virtual bool ReqInstrumentInfo(const utils::InstrumtntID &ins) = 0;
  virtual bool LossConnection() = 0;
};

#endif