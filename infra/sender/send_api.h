#ifndef __MARKETSENDAPI__
#define __MARKETSENDAPI__
#include "common/self/utils.h"
#include "vector"

struct SendApi {
 public:
  virtual bool ReqUserLogin() = 0;
  virtual bool ReqUserLogout() = 0;
  virtual bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0) = 0;
  virtual bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0) = 0;
  virtual bool LossConnection() = 0;
  // 只有快速回测模式下集成实例化该接口
  virtual bool SetBacktestControl(const std::string &begin, const std::string &end, uint32_t speed, uint32_t source, uint32_t indication) {
    return true;
  };
  virtual ~SendApi(){};
};

#endif