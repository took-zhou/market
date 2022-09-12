/*
 * ctpviewEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/interface/ctpview_event/ctpview_event.h"
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/protobuf/ctpview-market.pb.h"
#include "common/self/protobuf/manage-market.pb.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"
#include "market/interface/market_event.h"

CtpviewEvent::CtpviewEvent() { RegMsgFun(); }

void CtpviewEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map.clear();
  msg_func_map["LoginControl"] = [this](utils::ItpMsg &msg) { LoginControlHandle(msg); };
  msg_func_map["CheckStrategyAlive"] = [this](utils::ItpMsg &msg) { CheckStrategyAliveHandle(msg); };
  msg_func_map["BlockControl"] = [this](utils::ItpMsg &msg) { BlockControlHandle(msg); };
  msg_func_map["BugInjection"] = [this](utils::ItpMsg &msg) { BugInjectionHandle(msg); };
  msg_func_map["SimulateMarketState"] = [this](utils::ItpMsg &msg) { SimulateMarketStateHandle(msg); };

  for (auto &iter : msg_func_map) {
    INFO_LOG("msg_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void CtpviewEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map.find(msg.msgName);
  if (iter != msg_func_map.end()) {
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
    auto ActiveSafetyFunc = [&]() { marketSer.ROLE(ActiveSafety).ReqAlive(); };
    INFO_LOG("ActiveSafetyFunc prepare ok");
    std::thread(ActiveSafetyFunc).detach();
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
    auto keyNameList = marketSer.ROLE(ControlPara).get_prid_list();
    for (auto &keyname : keyNameList) {
      strategy_market::message tick;
      auto market_state = tick.mutable_market_state();

      market_state->set_market_state(state);
      market_state->set_date(simulate_market_state.date());

      utils::ItpMsg msg;
      tick.SerializeToString(&msg.pbMsg);
      msg.sessionName = "strategy_market";
      msg.msgName = "TickMarketState." + keyname;
      auto &recerSender = RecerSender::getInstance();
      recerSender.ROLE(Sender).ROLE(ProxySender).Send(msg);

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
    utils::ItpMsg msg;
    tick.SerializeToString(&msg.pbMsg);
    msg.sessionName = "manage_market";
    msg.msgName = "TickMarketState.0000000000";
    auto &recerSender = RecerSender::getInstance();
    recerSender.ROLE(Sender).ROLE(ProxySender).Send(msg);
    std::this_thread::sleep_for(10ms);
  } else {
    ERROR_LOG("not find target: %s.", simulate_market_state.target().c_str());
  }
}
