#ifndef __MARKETSENDAPI__
#define __MARKETSENDAPI__
#include "common/self/utils.h"

struct SendApi {
 public:
  virtual bool ReqUserLogin() = 0;
  virtual bool ReqUserLogout() = 0;
  virtual bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0) = 0;
  virtual bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0) = 0;
  virtual bool LossConnection() = 0;
  virtual ~SendApi(){};
};

#endif