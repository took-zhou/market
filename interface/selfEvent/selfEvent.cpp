/*
 * selfEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/interface/selfEvent/selfEvent.h"
#include "common/extern/google/protobuf/text_format.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/market-market.pb.h"
#include "common/self/utils.h"
#include "market/infra/define.h"

bool SelfEvent::init() {
  regMsgFun();

  return true;
}

void SelfEvent::regMsgFun() {
  int cnt = 0;
  msgFuncMap.clear();
  msgFuncMap["HeartBeat"] = [this](MsgStruct &msg) { HeartBeatHandle(msg); };

  for (auto &iter : msgFuncMap) {
    INFO_LOG("msgFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
  return;
}

void SelfEvent::handle(MsgStruct &msg) {
  auto iter = msgFuncMap.find(msg.msgName);
  if (iter != msgFuncMap.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
  return;
}

void SelfEvent::HeartBeatHandle(MsgStruct &msg) {
  static market_market::message reqMsg;
  reqMsg.ParseFromString(msg.pbMsg);
  utils::printProtoMsg(reqMsg);
}
