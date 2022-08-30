#ifndef WORKSPACE_MARKET_INFRA_ITPSENDER_H_
#define WORKSPACE_MARKET_INFRA_ITPSENDER_H_

#include "market/infra/sender/sendApi.h"

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
  SendApi *sendApi = nullptr;
};

#endif