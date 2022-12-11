/*
 * btp_event.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INTERFACE_BTPEVENT_BTPEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_BTPEVENT_BTPEVENT_H_

#include <functional>
#include <map>
#include <string>
#include "common/extern/btp/inc/btp_market_struct.h"
#include "common/self/utils.h"

struct BtpEvent {
 public:
  BtpEvent();

  void Handle(utils::ItpMsg &msg);
  void RegMsgFun();

  // 处理深度行情数据
  void OnDepthMarketDataHandle(utils::ItpMsg &msg);

  // 处理登录事件处理
  void OnRspUserLoginHandle(utils::ItpMsg &msg);

  // 处理登出事件处理
  void OnRspUserLogoutHandle(utils::ItpMsg &msg);

  void OnRspAllInstrumentInfoHandle(utils::ItpMsg &msg);

  std::map<std::string, std::function<void(utils::ItpMsg &msg)>> msg_func_map;
};

#endif /* WORKSPACE_MARKET_INTERFACE_BTPEVENT_BTPEVENT_H_ */
