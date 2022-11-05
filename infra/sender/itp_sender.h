#ifndef WORKSPACE_MARKET_INFRA_ITPSENDER_H_
#define WORKSPACE_MARKET_INFRA_ITPSENDER_H_

#include "market/infra/sender/send_api.h"

struct ItpSender {
 public:
  ItpSender();
  ItpSender(const ItpSender &) = delete;
  ItpSender &operator=(const ItpSender &) = delete;
  static ItpSender &GetInstance() {
    static ItpSender instance;
    return instance;
  }

  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool LossConnection();

 private:
  SendApi *send_api_ = nullptr;
};

#endif