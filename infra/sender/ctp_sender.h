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
  ~CtpSender(void);
  bool ReqUserLogin();
  bool ReqUserLogout();
  bool SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id = 0);
  bool LossConnection();

  bool Init(void);
  bool Release(void);

 private:
  static CThostFtdcMdApi *market_api;
  static CtpMarketSpi *market_spi;

  std::string con_path_ = "";
  int request_id_ = 0;
  bool is_init_ = false;
};

#endif /* WORKSPACE_MARKET_INFRA_CTPSENDER_H_ */
