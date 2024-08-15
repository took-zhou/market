/*
 * strategyEvent.cpp
 */
#include <fstream>
#include <iomanip>
#include <iostream>

#include "common/extern/log/log.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/utils.h"
#include "market/infra/recer_sender.h"
#include "market/interface/self_event/self_event.h"

SelfEvent::SelfEvent() { RegMsgFun(); }

void SelfEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map_.find(msg.msg_name);
  if (iter != msg_func_map_.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msg_name [%s]!", msg.msg_name.c_str());
  return;
}

void SelfEvent::RegMsgFun() {
  msg_func_map_.clear();
  msg_func_map_["SendEmail"] = [this](utils::ItpMsg &msg) { SendEmailHandle(msg); };
}

void SelfEvent::SendEmailHandle(utils::ItpMsg &msg) {
  auto &recer_sender = RecerSender::GetInstance();
  ipc::message recv_message;
  recv_message.ParseFromString(msg.pb_msg);
  auto &head = recv_message.send_email().head();
  auto &body = recv_message.send_email().body();
  recer_sender.ROLE(Sender).ROLE(EmailSender).Send(head.c_str(), body.c_str());
}