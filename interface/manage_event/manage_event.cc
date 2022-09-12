/*
 * manageEvent.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/interface/manage_event/manage_event.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/manage-market.pb.h"
#include "market/domain/market_service.h"
#include "market/interface/market_event.h"

ManageEvent::ManageEvent() { RegMsgFun(); }

void ManageEvent::RegMsgFun() {
  int cnt = 0;
  msg_func_map.clear();
  msg_func_map["TickMarketStateReq"] = [this](utils::ItpMsg &msg) { TickMarketStateReqReqHandle(msg); };

  for (auto &iter : msg_func_map) {
    INFO_LOG("msg_func_map[%d] key is [%s]", cnt, iter.first.c_str());
    cnt++;
  }
}

void ManageEvent::Handle(utils::ItpMsg &msg) {
  auto iter = msg_func_map.find(msg.msgName);
  if (iter != msg_func_map.end()) {
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
    marketSer.ROLE(PublishState).publish_to_manage();
  }
}
