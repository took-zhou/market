#include "market/domain/components/subscribe_manage.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"

SubscribeManager::SubscribeManager() {
  subscribed.instrumentIDs.clear();
  pthread_mutex_init(&(subscribed.sm_mutex), NULL);
}

void SubscribeManager::reqInstrumentsFromLocal() {
  std::vector<utils::InstrumtntID> insVec;
  utils::InstrumtntID ins;
  ins.ins = "002236";
  ins.exch = "SZSE";
  insVec.push_back(ins);

  insVec = remove_subscribed(insVec);
  auto& recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ItpSender).SubscribeMarketData(insVec);
}

void SubscribeManager::reqInstrumrntFromControlPara() {
  auto& marketSer = MarketService::getInstance();
  auto insVec = marketSer.ROLE(ControlPara).get_instrument_list();

  insVec = remove_subscribed(insVec);
  auto& recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ItpSender).SubscribeMarketData(insVec);
}

void SubscribeManager::reqInstrumentsFromMarket() {
  utils::InstrumtntID ins;

  ins.ins = "*";
  ins.exch = "SHSE";
  auto& recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ItpSender).ReqInstrumentInfo(ins);

  ins.ins = "*";
  ins.exch = "SZSE";
  recerSender.ROLE(Sender).ROLE(ItpSender).ReqInstrumentInfo(ins);
}

void SubscribeManager::reqInstrumentsFromTrader() {
  market_trader::message reqMsg;
  auto reqInstrument = reqMsg.mutable_qry_instrument_req();
  reqInstrument->set_identity("all");

  utils::ItpMsg msg;
  reqMsg.SerializeToString(&msg.pbMsg);
  msg.sessionName = "market_trader";
  msg.msgName = "QryInstrumentReq";
  auto& recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).Send(msg);
}

void SubscribeManager::subscribeInstrument(std::vector<utils::InstrumtntID>& nameVec) {
  auto& marketSer = MarketService::getInstance();

  auto& recerSender = RecerSender::getInstance();
  nameVec = remove_subscribed(nameVec);
  recerSender.ROLE(Sender).ROLE(ItpSender).SubscribeMarketData(nameVec);
}

void SubscribeManager::unSubscribeInstrument(std::vector<utils::InstrumtntID>& nameVec) {
  auto& recerSender = RecerSender::getInstance();
  nameVec = remove_unsubscribed(nameVec);
  recerSender.ROLE(Sender).ROLE(ItpSender).UnSubscribeMarketData(nameVec);
}

void SubscribeManager::unSubscribeAll() {
  int count = 0;
  std::vector<utils::InstrumtntID> insVec;

  for (auto iter = subscribed.instrumentIDs.begin(); iter != subscribed.instrumentIDs.end(); iter++) {
    insVec.push_back(*iter);
    count++;
  }

  insVec = remove_subscribed(insVec);
  auto& recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ItpSender).UnSubscribeMarketData(insVec);
  INFO_LOG("The number of contracts being unsubscribe is: %d.", count);
}

const std::vector<utils::InstrumtntID> SubscribeManager::remove_subscribed(std::vector<utils::InstrumtntID>& nameVec) {
  int ik = pthread_mutex_lock(&(subscribed.sm_mutex));

  for (auto iter = nameVec.begin(); iter != nameVec.end();) {
    if (subscribed.instrumentIDs.find(*iter) == end(subscribed.instrumentIDs)) {
      subscribed.instrumentIDs.insert(*iter);
      iter++;
    } else {
      iter = nameVec.erase(iter);
    }
  }
  ik = pthread_mutex_unlock(&(subscribed.sm_mutex));
  return nameVec;
}

const std::vector<utils::InstrumtntID> SubscribeManager::remove_unsubscribed(std::vector<utils::InstrumtntID>& nameVec) {
  int ik = pthread_mutex_lock(&(subscribed.sm_mutex));
  for (auto iter = nameVec.begin(); iter != nameVec.end();) {
    if (subscribed.instrumentIDs.find(*iter) == end(subscribed.instrumentIDs)) {
      iter = nameVec.erase(iter);
    } else {
      subscribed.instrumentIDs.erase(*iter);
      iter++;
    }
  }
  ik = pthread_mutex_unlock(&(subscribed.sm_mutex));

  return nameVec;
}