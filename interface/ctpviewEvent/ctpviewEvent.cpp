/*
 * ctpviewEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/interface/ctpviewEvent/ctpviewEvent.h"
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/protobuf/ctpview-market.pb.h"
#include "common/self/protobuf/manage-market.pb.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "market/domain/marketService.h"
#include "market/infra/recerSender.h"
#include "market/interface/marketEvent.h"

CtpviewEvent::CtpviewEvent() { regMsgFun(); }

void CtpviewEvent::regMsgFun() {
  int cnt = 0;
  msgFuncMap.clear();
  msgFuncMap["LoginControl"] = [this](utils::ItpMsg &msg) { LoginControlHandle(msg); };
  msgFuncMap["CheckStrategyAlive"] = [this](utils::ItpMsg &msg) { CheckStrategyAliveHandle(msg); };
  msgFuncMap["BlockControl"] = [this](utils::ItpMsg &msg) { BlockControlHandle(msg); };
  msgFuncMap["BugInjection"] = [this](utils::ItpMsg &msg) { BugInjectionHandle(msg); };
  msgFuncMap["SimulateMarketState"] = [this](utils::ItpMsg &msg) { SimulateMarketStateHandle(msg); };

  for (auto &iter : msgFuncMap) {
    INFO_LOG("msgFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void CtpviewEvent::handle(utils::ItpMsg &msg) {
  auto iter = msgFuncMap.find(msg.msgName);
  if (iter != msgFuncMap.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
  return;
}

void CtpviewEvent::LoginControlHandle(utils::ItpMsg &msg) {
  ctpview_market::message login_control;
  login_control.ParseFromString(msg.pbMsg);
  auto indication = login_control.login_control();

  int command = indication.command();
  auto &marketSer = MarketService::getInstance();

  INFO_LOG("force set time state: %d", command);
  marketSer.ROLE(MarketTimeState).set_time_state(command);
}

void CtpviewEvent::CheckStrategyAliveHandle(utils::ItpMsg &msg) {
  string command = "";
  ctpview_market::message check_alive;
  check_alive.ParseFromString(msg.pbMsg);
  auto indication = check_alive.check_alive();

  command = indication.check();
  auto &marketSer = MarketService::getInstance();

  if (command == "yes") {
    // 开启检测合约是否存活线程
    auto activeSafetyFunc = [&]() { marketSer.ROLE(activeSafety).req_alive(); };
    INFO_LOG("activeSafetyFunc prepare ok");
    std::thread(activeSafetyFunc).detach();
  }
}

void CtpviewEvent::BlockControlHandle(utils::ItpMsg &msg) {
  ctpview_market::message block_control;
  block_control.ParseFromString(msg.pbMsg);
  auto indication = block_control.block_control();

  ctpview_market::BlockControl_Command command = indication.command();
  auto &marketEvent = MarketEvent::getInstance();

  INFO_LOG("set block quotation control: %d", command);
  marketEvent.ROLE(CtpEvent).set_block_control(command);
}

void CtpviewEvent::BugInjectionHandle(utils::ItpMsg &msg) {
  ctpview_market::message bug_injection;
  bug_injection.ParseFromString(msg.pbMsg);
  auto injection = bug_injection.bug_injection();

  INFO_LOG("%s", msg.pbMsg.c_str());
  ctpview_market::BugInjection_InjectionType type = injection.type();
  INFO_LOG("set bug injection type: %d", type);

  if (type == ctpview_market::BugInjection_InjectionType_double_free) {
    int *a = new int;
    std::shared_ptr<int> p(a);
    delete a;
  }
}

void CtpviewEvent::SimulateMarketStateHandle(utils::ItpMsg &msg) {
  ctpview_market::message _simulate_market_state;
  _simulate_market_state.ParseFromString(msg.pbMsg);
  auto simulate_market_state = _simulate_market_state.simulate_market_state();

  if (simulate_market_state.target() == "strategy") {
    strategy_market::TickMarketState_MarketState state = strategy_market::TickMarketState_MarketState_reserve;
    if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_day_open) {
      state = strategy_market::TickMarketState_MarketState_day_open;
      INFO_LOG("Publish makret state: day_open, date: %s  to strategy.", simulate_market_state.date().c_str());
    } else if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_day_close) {
      state = strategy_market::TickMarketState_MarketState_day_close;
      INFO_LOG("Publish makret state: day_close, date: %s  to strategy.", simulate_market_state.date().c_str());
    } else if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_night_open) {
      state = strategy_market::TickMarketState_MarketState_night_open;
      INFO_LOG("Publish makret state: night_open, date: %s  to strategy.", simulate_market_state.date().c_str());
    } else if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_night_close) {
      state = strategy_market::TickMarketState_MarketState_night_close;
      INFO_LOG("Publish makret state: night_close, date: %s  to strategy.", simulate_market_state.date().c_str());
    }

    auto &marketSer = MarketService::getInstance();
    auto keyNameList = marketSer.ROLE(controlPara).getIdentifyList();
    for (auto &keyname : keyNameList) {
      strategy_market::message tick;
      auto market_state = tick.mutable_market_state();

      market_state->set_market_state(state);
      market_state->set_date(simulate_market_state.date());
      std::string tickStr;
      tick.SerializeToString(&tickStr);
      auto &recerSender = RecerSender::getInstance();
      string topic = "strategy_market.TickMarketState." + keyname;
      recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());
      std::this_thread::sleep_for(10ms);
    }

  } else if (simulate_market_state.target() == "manage") {
    manage_market::TickMarketState_MarketState state2 = manage_market::TickMarketState_MarketState_reserve;
    if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_day_open) {
      state2 = manage_market::TickMarketState_MarketState_day_open;
      INFO_LOG("Publish makret state: day_open, date: %s  to manage.", simulate_market_state.date().c_str());
    } else if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_day_close) {
      state2 = manage_market::TickMarketState_MarketState_day_close;
      INFO_LOG("Publish makret state: day_close, date: %s  to manage.", simulate_market_state.date().c_str());
    } else if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_night_open) {
      state2 = manage_market::TickMarketState_MarketState_night_open;
      INFO_LOG("Publish makret state: night_open, date: %s  to manage.", simulate_market_state.date().c_str());
    } else if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_night_close) {
      state2 = manage_market::TickMarketState_MarketState_night_close;
      INFO_LOG("Publish makret state: night_close, date: %s  to manage.", simulate_market_state.date().c_str());
    }

    manage_market::message tick;
    auto market_state = tick.mutable_market_state();

    market_state->set_market_state(state2);
    market_state->set_date(simulate_market_state.date());
    std::string tickStr;
    tick.SerializeToString(&tickStr);
    auto &recerSender = RecerSender::getInstance();
    string topic = "manage_market.TickMarketState.0000000000";
    recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());
    std::this_thread::sleep_for(10ms);
  } else {
    ERROR_LOG("not find target: %s.", simulate_market_state.target().c_str());
  }
}
