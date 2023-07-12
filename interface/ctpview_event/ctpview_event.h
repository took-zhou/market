/*
 * ctpview_event.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INTERFACE_CTPVIEWEVENT_CTPVIEWWEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_CTPVIEWEVENT_CTPVIEWWEVENT_H_

#include <functional>
#include <map>
#include <string>
#include "common/self/utils.h"

struct CtpviewEvent {
  CtpviewEvent();
  void Handle(utils::ItpMsg &msg);
  void RegMsgFun();

  void LoginControlHandle(utils::ItpMsg &msg);
  void BlockControlHandle(utils::ItpMsg &msg);
  void BugInjectionHandle(utils::ItpMsg &msg);
  void SimulateMarketStateHandle(utils::ItpMsg &msg);
  void TickStartStopIndicationHandle(utils::ItpMsg &msg);
  void BackTestControlHandle(utils::ItpMsg &msg);
  void ProfilerControlHandle(utils::ItpMsg &msg);
  void UpdateParaHandle(utils::ItpMsg &msg);

  std::map<std::string, std::function<void(utils::ItpMsg &msg)>> msg_func_map;
};

#endif /* WORKSPACE_MARKET_INTERFACE_CTPVIEWEVENT_CTPVIEWWEVENT_H_ */
