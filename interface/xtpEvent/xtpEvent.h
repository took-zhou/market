/*
 * ctpEvent.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INTERFACE_XTPEVENT_CTPEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_XTPEVENT_CTPEVENT_H_

#include <functional>
#include <map>
#include <string>
#include "common/self/protobuf/ctpview-market.pb.h"
#include "common/self/utils.h"

struct XtpEvent {
 public:
  XtpEvent();

  void handle(utils::ItpMsg &msg);
  void regMsgFun();

  // 处理深度行情数据
  void OnDepthMarketDataHandle(utils::ItpMsg &msg);

  // 处理登录事件处理
  void OnRspUserLoginHandle(utils::ItpMsg &msg);

  // 处理登出事件处理
  void OnRspUserLogoutHandle(utils::ItpMsg &msg);

  void OnQueryAllTickersHandle(utils::ItpMsg &msg);

  void set_block_control(ctpview_market::BlockControl_Command command);

  std::map<std::string, std::function<void(utils::ItpMsg &msg)>> msgFuncMap;

 private:
  std::string reqInstrumentFrom = "local";
  ctpview_market::BlockControl_Command block_control = ctpview_market::BlockControl_Command_unblock;
};

#endif /* WORKSPACE_MARKET_INTERFACE_XTPEVENT_CTPEVENT_H_ */
