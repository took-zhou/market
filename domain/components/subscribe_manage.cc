#include "market/domain/components/subscribe_manage.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"

SubscribeManager::SubscribeManager() {
  subscribed_.instrument_ids.clear();
  pthread_mutex_init(&(subscribed_.sm_mutex), NULL);
}

void SubscribeManager::ReqInstrumentsFromLocal() {
  std::vector<utils::InstrumtntID> ins_vec;
  utils::InstrumtntID ins;
  ins.ins = "002236";
  ins.exch = "SZSE";
  ins_vec.push_back(ins);

  ins_vec = RemoveSubscribed(ins_vec);
  auto& recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ItpSender).SubscribeMarketData(ins_vec);
}

void SubscribeManager::ReqInstrumrntFromControlPara() {
  auto& market_ser = MarketService::GetInstance();
  auto ins_vec = market_ser.ROLE(ControlPara).GetInstrumentList();

  ins_vec = RemoveSubscribed(ins_vec);
  auto& recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ItpSender).SubscribeMarketData(ins_vec);
}

void SubscribeManager::ReqInstrumentsFromMarket() {
  utils::InstrumtntID ins;

  ins.ins = "*";
  ins.exch = "SHSE";
  auto& recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ItpSender).ReqInstrumentInfo(ins);

  ins.ins = "*";
  ins.exch = "SZSE";
  recer_sender.ROLE(Sender).ROLE(ItpSender).ReqInstrumentInfo(ins);
}

void SubscribeManager::ReqInstrumentsFromTrader() {
  market_trader::message req_msg;
  auto req_instrument = req_msg.mutable_qry_instrument_req();
  req_instrument->set_identity("all");

  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "market_trader";
  msg.msg_name = "QryInstrumentReq";
  auto& recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);
}

void SubscribeManager::SubscribeInstrument(std::vector<utils::InstrumtntID>& name_vec) {
  auto& market_ser = MarketService::GetInstance();

  auto& recer_sender = RecerSender::GetInstance();
  name_vec = RemoveSubscribed(name_vec);
  recer_sender.ROLE(Sender).ROLE(ItpSender).SubscribeMarketData(name_vec);
}

void SubscribeManager::UnSubscribeInstrument(std::vector<utils::InstrumtntID>& name_vec) {
  auto& recer_sender = RecerSender::GetInstance();
  name_vec = RemoveSubscribed(name_vec);
  recer_sender.ROLE(Sender).ROLE(ItpSender).UnSubscribeMarketData(name_vec);
}

void SubscribeManager::UnSubscribeAll() {
  int count = 0;
  std::vector<utils::InstrumtntID> ins_vec;

  for (auto iter = subscribed_.instrument_ids.begin(); iter != subscribed_.instrument_ids.end(); iter++) {
    ins_vec.push_back(*iter);
    count++;
  }

  ins_vec = RemoveSubscribed(ins_vec);
  auto& recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ItpSender).UnSubscribeMarketData(ins_vec);
  INFO_LOG("The number of contracts being unsubscribe is: %d.", count);
}

const std::vector<utils::InstrumtntID> SubscribeManager::RemoveSubscribed(std::vector<utils::InstrumtntID>& name_vec) {
  int i_k = pthread_mutex_lock(&(subscribed_.sm_mutex));

  for (auto iter = name_vec.begin(); iter != name_vec.end();) {
    if (subscribed_.instrument_ids.find(*iter) == end(subscribed_.instrument_ids)) {
      subscribed_.instrument_ids.insert(*iter);
      iter++;
    } else {
      iter = name_vec.erase(iter);
    }
  }
  i_k = pthread_mutex_unlock(&(subscribed_.sm_mutex));
  return name_vec;
}

const std::vector<utils::InstrumtntID> SubscribeManager::RemoveUnsubscribed(std::vector<utils::InstrumtntID>& name_vec) {
  int i_k = pthread_mutex_lock(&(subscribed_.sm_mutex));
  for (auto iter = name_vec.begin(); iter != name_vec.end();) {
    if (subscribed_.instrument_ids.find(*iter) == end(subscribed_.instrument_ids)) {
      iter = name_vec.erase(iter);
    } else {
      subscribed_.instrument_ids.erase(*iter);
      iter++;
    }
  }
  i_k = pthread_mutex_unlock(&(subscribed_.sm_mutex));

  return name_vec;
}