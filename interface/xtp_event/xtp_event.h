/*
 * ctp_event.h
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

  void Handle(utils::ItpMsg &msg);
  void RegMsgFun();

  // 处理深度行情数据
  void OnDepthMarketDataHandle(utils::ItpMsg &msg);

  // 处理登录事件处理
  void OnRspUserLoginHandle(utils::ItpMsg &msg);

  // 处理登出事件处理
  void OnRspUserLogoutHandle(utils::ItpMsg &msg);

  void OnQueryAllTickersHandle(utils::ItpMsg &msg);

  void set_block_control(ctpview_market::BlockControl_Command command);

  std::map<std::string, std::function<void(utils::ItpMsg &msg)>> msg_func_map;

 private:
  std::string req_instrument_from = "local";
  ctpview_market::BlockControl_Command block_control = ctpview_market::BlockControl_Command_unblock;
};

#endif /* WORKSPACE_MARKET_INTERFACE_XTPEVENT_CTPEVENT_H_ */
