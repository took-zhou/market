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
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/semaphore.h"
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
  msg_func_map["TickStartStopIndication"] = [this](utils::ItpMsg &msg) { TickStartStopIndicationHandle(msg); };
  msg_func_map["BackTestControl"] = [this](utils::ItpMsg &msg) { BackTestControlHandle(msg); };

  for (auto &iter : msg_func_map) {
    INFO_LOG("msg_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void CtpviewEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map.find(msg.msg_name);
  if (iter != msg_func_map.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msg_name [%s]!", msg.msg_name.c_str());
  return;
}

void CtpviewEvent::LoginControlHandle(utils::ItpMsg &msg) {
  ctpview_market::message login_control;
  login_control.ParseFromString(msg.pb_msg);
  auto indication = login_control.login_control();

  int command = indication.command();
  auto &market_ser = MarketService::GetInstance();

  INFO_LOG("force set time state: %d", command);
  market_ser.ROLE(MarketTimeState).SetTimeState(command);
}

void CtpviewEvent::CheckStrategyAliveHandle(utils::ItpMsg &msg) {
  string command = "";
  ctpview_market::message check_alive;
  check_alive.ParseFromString(msg.pb_msg);
  auto indication = check_alive.check_alive();

  command = indication.check();
  auto &market_ser = MarketService::GetInstance();

  if (command == "yes") {
    // 开启检测合约是否存活线程
    auto active_safety_func = [&]() { market_ser.ROLE(ActiveSafety).ReqAlive(); };
    INFO_LOG("ActiveSafetyFunc prepare ok");
    std::thread(active_safety_func).detach();
  }
}

void CtpviewEvent::BlockControlHandle(utils::ItpMsg &msg) {
  ctpview_market::message block_control;
  block_control.ParseFromString(msg.pb_msg);
  auto indication = block_control.block_control();

  ctpview_market::BlockControl_Command command = indication.command();
  auto &market_event = MarketEvent::GetInstance();

  INFO_LOG("set block quotation control: %d", command);
  market_event.ROLE(CtpEvent).SetBlockControl(command);
}

void CtpviewEvent::BugInjectionHandle(utils::ItpMsg &msg) {
  ctpview_market::message bug_injection;
  bug_injection.ParseFromString(msg.pb_msg);
  auto injection = bug_injection.bug_injection();

  INFO_LOG("%s", msg.pb_msg.c_str());
  ctpview_market::BugInjection_InjectionType type = injection.type();
  INFO_LOG("set bug injection type: %d", type);

  if (type == ctpview_market::BugInjection_InjectionType_double_free) {
    int *tempa = new int;
    std::shared_ptr<int> ptr(tempa);
    delete tempa;
  }
}

void CtpviewEvent::SimulateMarketStateHandle(utils::ItpMsg &msg) {
  ctpview_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto simulate_market_state = message.simulate_market_state();

  strategy_market::MarketStateReq_MarketState state = strategy_market::MarketStateReq_MarketState_reserve;
  if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_day_open) {
    state = strategy_market::MarketStateReq_MarketState_day_open;
    INFO_LOG("Publish makret state: day_open, date: %s  to strategy.", simulate_market_state.date().c_str());
  } else if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_day_close) {
    state = strategy_market::MarketStateReq_MarketState_day_close;
    INFO_LOG("Publish makret state: day_close, date: %s  to strategy.", simulate_market_state.date().c_str());
  } else if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_night_open) {
    state = strategy_market::MarketStateReq_MarketState_night_open;
    INFO_LOG("Publish makret state: night_open, date: %s  to strategy.", simulate_market_state.date().c_str());
  } else if (simulate_market_state.market_state() == ctpview_market::SimulateMarketState_MarketState_night_close) {
    state = strategy_market::MarketStateReq_MarketState_night_close;
    INFO_LOG("Publish makret state: night_close, date: %s  to strategy.", simulate_market_state.date().c_str());
  }

  auto &market_ser = MarketService::GetInstance();
  auto key_name_list = market_ser.ROLE(ControlPara).GetPridList();
  for (auto &keyname : key_name_list) {
    strategy_market::message tick;
    auto market_state = tick.mutable_market_state_req();

    market_state->set_market_state(state);
    market_state->set_date(simulate_market_state.date());

    utils::ItpMsg msg;
    tick.SerializeToString(&msg.pb_msg);
    msg.session_name = "strategy_market";
    msg.msg_name = "MarketStateReq." + keyname;
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);

    auto &global_sem = GlobalSem::GetInstance();
    global_sem.WaitSemBySemName(GlobalSem::kStrategyRsp, 60);
  }
}

void CtpviewEvent::TickStartStopIndicationHandle(utils::ItpMsg &msg) {
  ctpview_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto indication = message.tick_start_stop_indication();

  std::string prid = indication.process_random_id();
  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(BacktestControl).SetStartStopIndication(prid, indication.type());
  if (indication.type() == ctpview_market::TickStartStopIndication_MessageType_start) {
    if (market_ser.login_state == kLoginState) {
      auto ins_vec = market_ser.ROLE(PublishControl).GetInstrumentList(prid);
      if (prid == "") {
        market_ser.ROLE(SubscribeManager).SubscribeInstrument(ins_vec);
      } else {
        market_ser.ROLE(SubscribeManager).SubscribeInstrument(ins_vec, stoi(prid));
      }
    } else {
      WARNING_LOG("now is logout, wait login to subscribe new instruments");
    }
  } else if (indication.type() == ctpview_market::TickStartStopIndication_MessageType_finish) {
    if (market_ser.login_state == kLoginState) {
      auto ins_vec = market_ser.ROLE(PublishControl).GetInstrumentList(prid);
      vector<utils::InstrumtntID> temp_ins_vec;
      for (auto &item : ins_vec) {
        if (market_ser.ROLE(PublishControl).GetInstrumentSubscribedCount(item.ins) == 1) {
          temp_ins_vec.push_back(item);
        }
      }
      if (prid == "") {
        market_ser.ROLE(SubscribeManager).UnSubscribeInstrument(temp_ins_vec);
      } else {
        market_ser.ROLE(SubscribeManager).UnSubscribeInstrument(temp_ins_vec, stoi(prid));
      }
    } else {
      WARNING_LOG("now is logout, wait login to unsubscribe new instruments");
    }
    market_ser.ROLE(BacktestControl).EraseControlPara(prid);
  }
}

void CtpviewEvent::BackTestControlHandle(utils::ItpMsg &msg) {
  ctpview_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto indication = message.backtest_control();

  BacktestPara b_p;
  std::string prid = indication.process_random_id();
  b_p.begin = indication.begin_time();
  b_p.end = indication.end_time();
  b_p.now = b_p.begin;
  b_p.speed = indication.speed();
  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(BacktestControl).BuildControlPara(prid, b_p);
}
