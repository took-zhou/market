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
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"
#include "market/interface/market_event.h"

CtpviewEvent::CtpviewEvent() { RegMsgFun(); }

void CtpviewEvent::RegMsgFun() {
  msg_func_map_.clear();
  msg_func_map_["LoginControl"] = [this](utils::ItpMsg &msg) { LoginControlHandle(msg); };
  msg_func_map_["BlockControl"] = [this](utils::ItpMsg &msg) { BlockControlHandle(msg); };
  msg_func_map_["BugInjection"] = [this](utils::ItpMsg &msg) { BugInjectionHandle(msg); };
  msg_func_map_["ProfilerControl"] = [this](utils::ItpMsg &msg) { ProfilerControlHandle(msg); };
  msg_func_map_["UpdatePara"] = [this](utils::ItpMsg &msg) { UpdateParaHandle(msg); };
  msg_func_map_["ClearDiagnosticEvent"] = [this](utils::ItpMsg &msg) { ClearDiagnosticEventHandle(msg); };
  msg_func_map_["SendTestEmail"] = [this](utils::ItpMsg &msg) { SendTestEmailHandle(msg); };
  msg_func_map_["ShareSplit"] = [this](utils::ItpMsg &msg) { ShareSplitHandle(msg); };
}

void CtpviewEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map_.find(msg.msg_name);
  if (iter != msg_func_map_.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msg name [%s]!", msg.msg_name.c_str());
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
  market_ser.ROLE(MarketTimeState).SetSubTimeState(command);
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
    market_event.ROLE(BtpEvent).SetBlockControl(ins, command);
    market_event.ROLE(CtpEvent).SetBlockControl(ins, command);
    market_event.ROLE(GtpEvent).SetBlockControl(ins, command);
    market_event.ROLE(YtpEvent).SetBlockControl(ins, command);
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

void CtpviewEvent::SendTestEmailHandle(utils::ItpMsg &msg) {
  ctpview_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto send_email = message.send_email();

  auto action = send_email.send_action();
  if (action == ctpview_market::SendTestEmail::send) {
    char subject_content[30] = "test email connection";
    char save_content[180] = "Test email connectivity without replying";

    auto &recer_sender = RecerSender::GetInstance();
    ipc::message send_message;
    auto *send_email = send_message.mutable_send_email();
    send_email->set_head(subject_content);
    send_email->set_body(save_content);

    utils::ItpMsg itp_msg;
    send_message.SerializeToString(&itp_msg.pb_msg);
    itp_msg.session_name = "market_market";
    itp_msg.msg_name = "SendEmail";
    // innerSenders专为itp设计，所以只能走ProxySender的接口
    recer_sender.ROLE(Sender).ROLE(ProxySender).SendMsg(itp_msg);
  }
}

void CtpviewEvent::ShareSplitHandle(utils::ItpMsg &msg) {
  ctpview_market::message message;
  message.ParseFromString(msg.pb_msg);
  auto share_split = message.share_split();

  ctpview_market::ShareSplit_Command command = share_split.command();
  auto &market_event = MarketEvent::GetInstance();

  for (int i = 0; i < share_split.instrument_size(); i++) {
    const auto &ins = share_split.instrument(i);
    INFO_LOG("set share split control: %s, %d", ins.c_str(), command);
    market_event.ROLE(FtpEvent).SetSplitControl(ins, command);
  }
}
