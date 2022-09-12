/*
 * MarketEvent.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INTERFACE_MARKETEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_MARKETEVENT_H_

#include "market/interface/ctp_event/ctp_event.h"
#include "market/interface/ctpview_event/ctpview_event.h"
#include "market/interface/manage_event/manage_event.h"
#include "market/interface/strategy_event/strategy_event.h"
#include "market/interface/trader_event/trader_event.h"
#include "market/interface/xtp_event/xtp_event.h"

#include <functional>
#include <map>
#include <string>
#include "common/self/dci/role.h"

struct MarketEvent : CtpEvent, XtpEvent, StrategyEvent, ManageEvent, TraderEvent, CtpviewEvent {
  MarketEvent();
  MarketEvent(const MarketEvent &) = delete;
  MarketEvent &operator=(const MarketEvent &) = delete;
  static MarketEvent &getInstance() {
    static MarketEvent instance;
    return instance;
  }

  bool Run();
  void RegSessionFunc();
  IMPL_ROLE(CtpEvent);
  IMPL_ROLE(XtpEvent);
  IMPL_ROLE(StrategyEvent);
  IMPL_ROLE(ManageEvent);
  IMPL_ROLE(TraderEvent);
  IMPL_ROLE(CtpviewEvent);
  std::map<std::string, std::function<void(utils::ItpMsg msg)>> session_func_map;
};

#endif /* WORKSPACE_MARKET_INTERFACE_MARKETEVENT_H_ */
