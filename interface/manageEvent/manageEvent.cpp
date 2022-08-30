/*
 * manageEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/interface/manageEvent/manageEvent.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/manage-market.pb.h"
#include "market/domain/marketService.h"
#include "market/interface/marketEvent.h"

ManageEvent::ManageEvent() { regMsgFun(); }

void ManageEvent::regMsgFun() {
  int cnt = 0;
  msgFuncMap.clear();
  msgFuncMap["TickMarketStateReq"] = [this](utils::ItpMsg &msg) { TickMarketStateReqReqHandle(msg); };

  for (auto &iter : msgFuncMap) {
    INFO_LOG("msgFuncMap[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void ManageEvent::handle(utils::ItpMsg &msg) {
  auto iter = msgFuncMap.find(msg.msgName);
  if (iter != msgFuncMap.end()) {
    iter->second(msg);
    return;
  }
  ERROR_LOG("can not find func for msgName [%s]!", msg.msgName.c_str());
  return;
}

void ManageEvent::TickMarketStateReqReqHandle(utils::ItpMsg &msg) {
  manage_market::message _req;
  _req.ParseFromString(msg.pbMsg);
  auto req = _req.market_state_req();

  std::string isreq = req.req();

  INFO_LOG("set market state req: %s", isreq.c_str());

  if (isreq == "yes") {
    auto &marketSer = MarketService::getInstance();
    marketSer.ROLE(publishState).publish_to_manage();
  }
}
