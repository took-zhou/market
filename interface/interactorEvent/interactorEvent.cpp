/*
 * interactorEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/interface/interactorEvent/interactorEvent.h"
#include "common/extern/log/log.h"
#include "market/infra/define.h"

bool InteractEvent::init() {
  regMsgFun();

  return true;
}

void InteractEvent::regMsgFun() {
  int cnt = 0;
  msgFuncMap.clear();

  for (auto iter : msgFuncMap) {
    INFO_LOG("msgFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void InteractEvent::handle(MsgStruct &msg) {
  auto iter = msgFuncMap.find(msg.msgName);
  if (iter != msgFuncMap.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
  return;
}
