/*
 * ctpviewEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/interface/ctpview_event/ctpview_event.h"
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/profiler.h"
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
  msg_func_map["BlockControl"] = [this](utils::ItpMsg &msg) { BlockControlHandle(msg); };
  msg_func_map["BugInjection"] = [this](utils::ItpMsg &msg) { BugInjectionHandle(msg); };
  msg_func_map["SimulateMarketState"] = [this](utils::ItpMsg &msg) { SimulateMarketStateHandle(msg); };
  msg_func_map["TickStartStopIndication"] = [this](utils::ItpMsg &msg) { TickStartStopIndicationHandle(msg); };
  msg_func_map["BackTestControl"] = [this](utils::ItpMsg &msg) { BackTestControlHandle(msg); };
  msg_func_map["ProfilerControl"] = [this](utils::ItpMsg &msg) { ProfilerControlHandle(msg); };
  msg_func_map["UpdatePara"] = [this](utils::ItpMsg &msg) { UpdateParaHandle(msg); };

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

void CtpviewEvent::BlockControlHandle(utils::ItpMsg &msg) {
  ctpview_market::message block_control;
  block_control.ParseFromString(msg.pb_msg);
  auto indication = block_control.block_control();

  ctpview_market::BlockControl_Command command = indication.command();
  auto &market_event = MarketEvent::GetInstance();

  for (int i = 0; i < indication.instrument_size(); i++) {
    auto ins = indication.instrument(i);
    INFO_LOG("set block quotation control: %s, %d", ins.c_str(), command);
    market_event.ROLE(CtpEvent).SetBlockControl(ins, command);
  }
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

  {
    auto &market_ser = MarketService::GetInstance();
    strategy_market::message tick;
    auto market_state = tick.mutable_market_state_req();
    market_state->set_market_state(state);
    market_state->set_date(simulate_market_state.date());
    market_state->set_is_last(1);

    utils::ItpMsg msg;
    tick.SerializeToString(&msg.pb_msg);
    msg.session_name = "strategy_market";
    msg.msg_name = "MarketStateReq";
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(Sender).ROLE(ProxySender).SendMsg(msg);
    market_ser.ROLE(PublishState).SetPublishFlag();
  }
}

void CtpviewEvent::TickStartStopIndicationHandle(utils::ItpMsg &msg) {
  ctpview_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto indication = message.tick_start_stop_indication();

  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(BacktestControl).SetStartStopIndication(indication.type());
}

void CtpviewEvent::BackTestControlHandle(utils::ItpMsg &msg) {
  ctpview_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto indication = message.backtest_control();

  BacktestPara b_p;
  b_p.begin = indication.begin_time();
  b_p.end = indication.end_time();
  b_p.speed = indication.speed();
  b_p.source = indication.source();
  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(BacktestControl).BuildControlPara(b_p);
}

void CtpviewEvent::ProfilerControlHandle(utils::ItpMsg &msg) {
  ctpview_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto control = message.profiler_control();

  auto action = control.profiler_action();
  if (action == ctpview_market::ProfilerControl::start_write) {
    profiler::FlameGraphWriter::Instance().StartAddData();
    INFO_LOG("start write tracepoint");
  } else if (action == ctpview_market::ProfilerControl::stop_write) {
    profiler::FlameGraphWriter::Instance().StopAddData();
    INFO_LOG("stop write tracepoint");
  }
}

void CtpviewEvent::UpdateParaHandle(utils::ItpMsg &msg) {
  ctpview_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto update = message.update_para();

  auto action = update.update_action();
  if (action == ctpview_market::UpdatePara::update) {
    utils::JsonConfig::GetInstance().GetConfig();
    INFO_LOG("reload config file");
  }
}
