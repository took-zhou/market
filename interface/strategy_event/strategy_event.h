/*
 * strategy_event.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INTERFACE_STRATEGYEVENT_STRATEGYEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_STRATEGYEVENT_STRATEGYEVENT_H_
#include <functional>
#include <map>
#include <string>
#include "common/self/utils.h"

struct StrategyEvent {
 public:
  StrategyEvent();

  void Handle(utils::ItpMsg &msg);
  void RegMsgFun();

  // 处理策略端请求的合约信息
  void TickSubscribeReqHandle(utils::ItpMsg &msg);

  // 策略是否运行回复处理
  void StrategyAliveRspHandle(utils::ItpMsg &msg);

  void TimeLimitReqHandle(utils::ItpMsg &msg);

  void InstrumentReqHandle(utils::ItpMsg &msg);

  void MarketStateRspHandle(utils::ItpMsg &msg);

  void InsertControlParaReqHandle(utils::ItpMsg &msg);

  void EraseControlParaReqHandle(utils::ItpMsg &msg);

 private:
  std::map<std::string, std::function<void(utils::ItpMsg &msg)>> msg_func_map_;
};

#endif /* WORKSPACE_MARKET_INTERFACE_STRATEGYEVENT_STRATEGYEVENT_H_ */
