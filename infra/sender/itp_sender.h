#ifndef WORKSPACE_MARKET_INFRA_ITPSENDER_H_
#define WORKSPACE_MARKET_INFRA_ITPSENDER_H_

#include "market/infra/sender/send_api.h"

struct ItpSender {
 public:
  ItpSender();
  ItpSender(const ItpSender &) = delete;
  ItpSender &operator=(const ItpSender &) = delete;
  static ItpSender &getInstance() {
    static ItpSender instance;
    return instance;
  }

  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec);
  bool ReqInstrumentInfo(const utils::InstrumtntID &ins);
  bool LossConnection();

 private:
  SendApi *send_api = nullptr;
};

#endif