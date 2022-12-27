/*
 * ctp_recer.h
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_CTPRECER_H_
#define WORKSPACE_MARKET_INFRA_CTPRECER_H_
#include "common/extern/ctp/inc/ThostFtdcMdApi.h"
#include "common/self/utils.h"

class CtpMarketSpi : public CThostFtdcMdSpi {
 public:
  virtual ~CtpMarketSpi() {}
  void OnFrontConnected();

  void OnFrontDisconnected(int reason);

  void OnHeartBeatWarning(int time_lapse);

  void OnRspUserLogin(CThostFtdcRspUserLoginField *rsp_user_login, CThostFtdcRspInfoField *rsp_info, int request_id, bool is_last);

  void OnRspUserLogout(CThostFtdcUserLogoutField *user_logout, CThostFtdcRspInfoField *rsp_info, int request_id, bool is_last);

  // market端ctp登出没有反馈，主动调用反馈接口
  void OnRspUserLogout(void);

  void OnRspError(CThostFtdcRspInfoField *rsp_info, int request_id, bool is_last){};

  void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *specific_instrument, CThostFtdcRspInfoField *rsp_info, int request_id,
                          bool is_last){};

  void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *specific_instrument, CThostFtdcRspInfoField *rsp_info, int request_id,
                            bool is_last){};

  void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *specific_instrument, CThostFtdcRspInfoField *rsp_info, int request_id,
                           bool is_last){};

  void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *specific_instrument, CThostFtdcRspInfoField *rsp_info, int request_id,
                             bool is_last){};

  void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *depth_market_data);

  void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *for_quote_rsp){};

  bool front_disconnected = false;

  int re_connect = 0;
};

#endif /* WORKSPACE_MARKET_INFRA_CTPRECER_H_ */
