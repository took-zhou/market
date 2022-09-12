/*
 * ctp_sender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_CTPSENDER_H_
#define WORKSPACE_MARKET_INFRA_CTPSENDER_H_

#include "common/extern/ctp/inc/ThostFtdcMdApi.h"
#include "common/self/utils.h"
#include "market/infra/recer/ctp_recer.h"
#include "market/infra/sender/send_api.h"

struct CtpSender : SendApi {
 public:
  CtpSender(void);
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec);
  bool ReqInstrumentInfo(const utils::InstrumtntID &ins);
  bool LossConnection();

  static CThostFtdcMdApi *kMarketApi;
  static CtpMarketSpi *kMarketSpi;

 private:
  bool Init(void);
  bool Release(void);

  std::string con_path = "";
  int request_id = 0;
  bool is_init = false;
};

#endif /* WORKSPACE_MARKET_INFRA_CTPSENDER_H_ */
