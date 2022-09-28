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

  AddSubscribed(ins_vec);
  auto& recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ItpSender).SubscribeMarketData(ins_vec);
}

void SubscribeManager::ReqInstrumrntFromControlPara() {
  auto& market_ser = MarketService::GetInstance();
  auto ins_vec = market_ser.ROLE(ControlPara).GetInstrumentList();

  AddSubscribed(ins_vec);
  auto& recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ItpSender).SubscribeMarketData(ins_vec);
}

void SubscribeManager::ReqInstrumentsFromApi() {
  static int instrument_count;
  static vector<utils::InstrumtntID> ins_vec;
  auto& market_server = MarketService::GetInstance();

  auto instrument_info = market_server.ROLE(InstrumentInfo).GetInstrumentList();
  for (auto& ins : instrument_info) {
    utils::InstrumtntID instrumtnt_id;

    instrumtnt_id.exch = market_server.ROLE(InstrumentInfo).GetExchange(ins);
    instrumtnt_id.ins = ins;

    ins_vec.push_back(instrumtnt_id);
    instrument_count++;

    if (ins_vec.size() >= 500) {
      SubscribeInstrument(ins_vec);
      ins_vec.clear();
    }
  }

  if (ins_vec.size() > 0) {
    SubscribeInstrument(ins_vec);
    INFO_LOG("The number of trading contracts is: %d.", instrument_count);
    instrument_count = 0;
    ins_vec.clear();
  }
}

void SubscribeManager::SubscribeInstrument(std::vector<utils::InstrumtntID>& name_vec, int request_id) {
  auto& recer_sender = RecerSender::GetInstance();
  AddSubscribed(name_vec);
  recer_sender.ROLE(Sender).ROLE(ItpSender).SubscribeMarketData(name_vec, request_id);
}

void SubscribeManager::UnSubscribeInstrument(std::vector<utils::InstrumtntID>& name_vec, int request_id) {
  auto& recer_sender = RecerSender::GetInstance();
  RemoveSubscribed(name_vec);
  recer_sender.ROLE(Sender).ROLE(ItpSender).UnSubscribeMarketData(name_vec, request_id);
}

void SubscribeManager::UnSubscribeAll() {
  int count = 0;
  std::vector<utils::InstrumtntID> ins_vec;

  auto& recer_sender = RecerSender::GetInstance();
  for (auto& item : subscribed_.instrument_ids) {
    ins_vec.push_back(item);
    if (ins_vec.size() >= 500) {
      recer_sender.ROLE(Sender).ROLE(ItpSender).UnSubscribeMarketData(ins_vec);
      ins_vec.clear();
    }
    count++;
  }
  if (ins_vec.size() != 0) {
    recer_sender.ROLE(Sender).ROLE(ItpSender).UnSubscribeMarketData(ins_vec);
    ins_vec.clear();
  }

  int i_k = pthread_mutex_lock(&(subscribed_.sm_mutex));
  subscribed_.instrument_ids.clear();
  i_k = pthread_mutex_unlock(&(subscribed_.sm_mutex));
  INFO_LOG("The number of contracts being unsubscribe is: %d.", count);
}

void SubscribeManager::AddSubscribed(std::vector<utils::InstrumtntID>& name_vec) {
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
}

void SubscribeManager::RemoveSubscribed(std::vector<utils::InstrumtntID>& name_vec) {
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
}