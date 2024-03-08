/*
 * ctp_event.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INTERFACE_CTPEVENT_CTPEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_CTPEVENT_CTPEVENT_H_

#include <functional>
#include <map>
#include <string>
#include "common/self/protobuf/ctpview-market.pb.h"
#include "common/self/utils.h"

struct BlockControl {
  bool block = false;
  std::set<std::string> instruments;
};

struct CtpEvent {
 public:
  CtpEvent();

  void Handle(utils::ItpMsg &msg);
  void RegMsgFun();

  // 处理深度行情数据
  void DeepMarktDataHandle(utils::ItpMsg &msg);

  // 处理登录事件处理
  void OnRspUserLoginHandle(utils::ItpMsg &msg);

  // 处理登出事件处理
  void OnRspUserLogoutHandle(utils::ItpMsg &msg);

  void SetBlockControl(const std::string &ins, ctpview_market::BlockControl_Command command);

 private:
  void UpdateInstrumentInfoFromTrader();
  std::map<std::string, std::function<void(utils::ItpMsg &msg)>> msg_func_map_;
  std::string req_instrument_from_ = "local";
  BlockControl block_control_;
};

#endif /* WORKSPACE_MARKET_INTERFACE_CTPEVENT_CTPEVENT_H_ */
