/*
 * MarketEvent.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INTERFACE_MARKETEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_MARKETEVENT_H_

#include "market/interface/btp_event/btp_event.h"
#include "market/interface/ctp_event/ctp_event.h"
#include "market/interface/ctpview_event/ctpview_event.h"
#include "market/interface/ftp_event/ftp_event.h"
#include "market/interface/gtp_event/gtp_event.h"
#include "market/interface/mtp_event/mtp_event.h"
#include "market/interface/otp_event/otp_event.h"
#include "market/interface/self_event/self_event.h"
#include "market/interface/strategy_event/strategy_event.h"
#include "market/interface/trader_event/trader_event.h"
#include "market/interface/xtp_event/xtp_event.h"

#include <functional>
#include <map>
#include <string>
#include <thread>
#include "common/self/dci/role.h"

struct MarketEvent : BtpEvent,
                     CtpEvent,
                     XtpEvent,
                     OtpEvent,
                     FtpEvent,
                     GtpEvent,
                     MtpEvent,
                     StrategyEvent,
                     TraderEvent,
                     CtpviewEvent,
                     SelfEvent {
 public:
  MarketEvent();
  ~MarketEvent();
  MarketEvent(const MarketEvent &) = delete;
  MarketEvent &operator=(const MarketEvent &) = delete;
  static MarketEvent &GetInstance() {
    static MarketEvent instance;
    return instance;
  }

  bool Run();
  void RegSessionFunc();
  IMPL_ROLE(BtpEvent);
  IMPL_ROLE(CtpEvent);
  IMPL_ROLE(XtpEvent);
  IMPL_ROLE(OtpEvent);
  IMPL_ROLE(FtpEvent);
  IMPL_ROLE(GtpEvent);
  IMPL_ROLE(MtpEvent);
  IMPL_ROLE(StrategyEvent);
  IMPL_ROLE(TraderEvent);
  IMPL_ROLE(CtpviewEvent);
  IMPL_ROLE(SelfEvent);

 private:
  void ProxyRecTask();
  void ItpRecTask();
  std::map<std::string, std::function<void(utils::ItpMsg msg)>> session_func_map_;
  std::thread proxy_rec_thread_;
  std::thread itp_rec_thread_;
  std::atomic<bool> running_{false};
};

#endif /* WORKSPACE_MARKET_INTERFACE_MARKETEVENT_H_ */
