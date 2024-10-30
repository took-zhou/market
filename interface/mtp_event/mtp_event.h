/*
 * mtp_event.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INTERFACE_MTPEVENT_MTPEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_MTPEVENT_MTPEVENT_H_

#include <functional>
#include <map>
#include <string>
#include "common/self/protobuf/ctpview-market.pb.h"
#include "common/self/utils.h"

struct MtpEvent {
 public:
  MtpEvent();

  void Handle(utils::ItpMsg &msg);
  void RegMsgFun();

  // 处理深度行情数据
  void OnDepthMarketDataHandle(utils::ItpMsg &msg);

  // 处理登录事件处理
  void OnRspUserLoginHandle(utils::ItpMsg &msg);

  // 处理登出事件处理
  void OnRspUserLogoutHandle(utils::ItpMsg &msg);

  void OnRspAllInstrumentInfoHandle(utils::ItpMsg &msg);

  void SetBlockControl(const std::string &ins, ctpview_market::BlockControl_Command command);

 private:
  struct BlockControl {
    bool block = false;
    std::set<std::string> instruments;
  };

  std::string req_instrument_from_ = "local";
  std::map<std::string, std::function<void(utils::ItpMsg &msg)>> msg_func_map_;
  BlockControl block_control_;
};

#endif /* WORKSPACE_MARKET_INTERFACE_BTPEVENT_MTPEVENT_H_ */
