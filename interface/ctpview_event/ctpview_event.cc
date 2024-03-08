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
#include "market/domain/market_service.h"
#include "market/interface/market_event.h"

CtpviewEvent::CtpviewEvent() { RegMsgFun(); }

void CtpviewEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map_.clear();
  msg_func_map_["LoginControl"] = [this](utils::ItpMsg &msg) { LoginControlHandle(msg); };
  msg_func_map_["BlockControl"] = [this](utils::ItpMsg &msg) { BlockControlHandle(msg); };
  msg_func_map_["BugInjection"] = [this](utils::ItpMsg &msg) { BugInjectionHandle(msg); };
  msg_func_map_["ProfilerControl"] = [this](utils::ItpMsg &msg) { ProfilerControlHandle(msg); };
  msg_func_map_["UpdatePara"] = [this](utils::ItpMsg &msg) { UpdateParaHandle(msg); };
  msg_func_map_["ClearDiagnosticEvent"] = [this](utils::ItpMsg &msg) { ClearDiagnosticEventHandle(msg); };

  for (auto &iter : msg_func_map_) {
    INFO_LOG("msg_func_map_[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void CtpviewEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map_.find(msg.msg_name);
  if (iter != msg_func_map_.end()) {
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
    const auto &ins = indication.instrument(i);
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

void CtpviewEvent::ClearDiagnosticEventHandle(utils::ItpMsg &msg) {
  auto &market_ser = MarketService::GetInstance();
  ctpview_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto clear_diagnostic_event = message.clear_diagnostic_event();
  auto event_id = clear_diagnostic_event.diagnostic_event_id();

  market_ser.ROLE(Diagnostic).ClearStatus(static_cast<DiagnosticEventId>(event_id));
}
