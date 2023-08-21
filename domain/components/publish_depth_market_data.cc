#include <thread>

//自定义头文件
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/profiler.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"
#include "market/domain/components/publish_depth_market_data.h"
#include "market/domain/market_service.h"
#include "market/infra/recer/ctp_recer.h"
#include "market/infra/recer_sender.h"

PublishData::PublishData() { ; }

void PublishData::DirectForwardDataToStrategy(CThostFtdcDepthMarketDataField *p_d) {
  PZone("DirectForwardDataToStrategy");
  auto &market_ser = MarketService::GetInstance();
  auto pos = market_ser.ROLE(PublishControl).publish_para_map.find(p_d->InstrumentID);
  if (pos != market_ser.ROLE(PublishControl).publish_para_map.end() && market_ser.login_state == kLoginState) {
    OnceFromDataflow(pos->second, p_d);
  } else if (pos == market_ser.ROLE(PublishControl).publish_para_map.end()) {
    ERROR_LOG("can not find ins from control para: %s", p_d->InstrumentID);
  }
}

void PublishData::OnceFromDataflow(const PublishPara &p_c, CThostFtdcDepthMarketDataField *p_d) {
  if (IsValidTickData(p_d) == false) {
    return;
  }

  char time_array[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  GetAssemblingTime(time_array, p_d);
  tick_data->set_time_point(time_array);

  tick_data->set_state(strategy_market::TickData_TickState_active);
  tick_data->set_instrument_id(p_d->InstrumentID);
  tick_data->set_last_price(Max2zero(p_d->LastPrice));
  tick_data->set_bid_price1(Max2zero(p_d->BidPrice1));
  tick_data->set_bid_volume1(p_d->BidVolume1);
  tick_data->set_ask_price1(Max2zero(p_d->AskPrice1));
  tick_data->set_ask_volume1(p_d->AskVolume1);
  if (kDataLevel_ == 2) {
    tick_data->set_bid_price2(Max2zero(p_d->BidPrice2));
    tick_data->set_bid_volume2(p_d->BidVolume2);
    tick_data->set_ask_price2(Max2zero(p_d->AskPrice2));
    tick_data->set_ask_volume2(p_d->AskVolume2);
    tick_data->set_bid_price3(Max2zero(p_d->BidPrice3));
    tick_data->set_bid_volume3(p_d->BidVolume3);
    tick_data->set_ask_price3(Max2zero(p_d->AskPrice3));
    tick_data->set_ask_volume3(p_d->AskVolume3);
    tick_data->set_bid_price4(Max2zero(p_d->BidPrice4));
    tick_data->set_bid_volume4(p_d->BidVolume4);
    tick_data->set_ask_price4(Max2zero(p_d->AskPrice4));
    tick_data->set_ask_volume4(p_d->AskVolume4);
    tick_data->set_bid_price5(Max2zero(p_d->BidPrice5));
    tick_data->set_bid_volume5(p_d->BidVolume5);
    tick_data->set_ask_price5(Max2zero(p_d->AskPrice5));
    tick_data->set_ask_volume5(p_d->AskVolume5);
  }

  tick_data->set_open_price(Max2zero(p_d->OpenPrice));
  tick_data->set_volume(p_d->Volume);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "TickData." + tick_data->instrument_id();
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(DirectSender).SendMsg(msg);

  p_c.heartbeat = 0;
}

void PublishData::HeartBeatDetect() {
  auto &market_ser = MarketService::GetInstance();
  auto &json_cfg = utils::JsonConfig::GetInstance();
  unsigned char entry_logoutstate_flag = false;

  if (market_ser.login_state == kLoginState) {
    for (auto &item_p_c : market_ser.ROLE(PublishControl).publish_para_map) {
      item_p_c.second.heartbeat++;
      if (item_p_c.second.heartbeat >= kHeartBeatWaitTime_ && json_cfg.GetConfig("market", "TimingPush") == "push") {
        OnceFromDefault(item_p_c.second, item_p_c.first);
      }
    }
    entry_logoutstate_flag = false;
  } else {
    if (entry_logoutstate_flag == false) {
      for (auto &item_p_c : market_ser.ROLE(PublishControl).publish_para_map) {
        item_p_c.second.heartbeat = 0;
      }
    }
    entry_logoutstate_flag = true;
  }
}

void PublishData::OnceFromDefault(const PublishPara &p_c, const string &ins) {
  auto &market_ser = MarketService::GetInstance();
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  auto timenow = market_ser.ROLE(MarketTimeState).GetTimeNow();
  char time_array[100] = {0};
  if (timenow != nullptr) {
    strftime(time_array, sizeof(time_array), "%Y-%m-%d %H:%M:%S.000", timenow);
  }
  tick_data->set_time_point(time_array);

  tick_data->set_state(strategy_market::TickData_TickState_inactive);
  tick_data->set_instrument_id(ins);
  tick_data->set_last_price(Max2zero(0.0));
  tick_data->set_bid_price1(Max2zero(0.0));
  tick_data->set_bid_volume1(0);
  tick_data->set_ask_price1(Max2zero(0.0));
  tick_data->set_ask_volume1(0);
  if (kDataLevel_ == 2) {
    tick_data->set_bid_price2(Max2zero(0.0));
    tick_data->set_bid_volume2(0);
    tick_data->set_ask_price2(Max2zero(0.0));
    tick_data->set_ask_volume2(0);
    tick_data->set_bid_price3(Max2zero(0.0));
    tick_data->set_bid_volume3(0);
    tick_data->set_ask_price3(Max2zero(0.0));
    tick_data->set_ask_volume3(0);
    tick_data->set_bid_price4(Max2zero(0.0));
    tick_data->set_bid_volume4(0);
    tick_data->set_ask_price4(Max2zero(0.0));
    tick_data->set_ask_volume4(0);
    tick_data->set_bid_price5(Max2zero(0.0));
    tick_data->set_bid_volume5(0);
    tick_data->set_ask_price5(Max2zero(0.0));
    tick_data->set_ask_volume5(0);
    tick_data->set_turnover(0);
    tick_data->set_open_interest(0);
    tick_data->set_upper_limit_price(Max2zero(0.0));
    tick_data->set_lower_limit_price(Max2zero(0.0));
    tick_data->set_pre_settlement_price(Max2zero(0.0));
    tick_data->set_pre_close_price(Max2zero(0.0));
    tick_data->set_pre_open_interest(0);
  }
  tick_data->set_open_price(Max2zero(0.0));

  tick_data->set_volume(0);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "TickData";
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).SendMsg(msg);

  p_c.heartbeat = 0;
}

void PublishData::DirectForwardDataToStrategy(XTPMD *p_d) {
  auto &market_ser = MarketService::GetInstance();
  auto pos = market_ser.ROLE(PublishControl).publish_para_map.find(p_d->ticker);
  if (pos != market_ser.ROLE(PublishControl).publish_para_map.end() && market_ser.login_state == kLoginState) {
    OnceFromDataflow(pos->second, p_d);
  } else if (pos == market_ser.ROLE(PublishControl).publish_para_map.end()) {
    ERROR_LOG("can not find ins from control para: %s", p_d->ticker);
  }
}

void PublishData::OnceFromDataflow(const PublishPara &p_c, XTPMD *p_d) {
  if (IsValidTickData(p_d) == false) {
    return;
  }

  char time_array[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  GetAssemblingTime(time_array, p_d);
  tick_data->set_time_point(time_array);

  tick_data->set_state(strategy_market::TickData_TickState_active);
  tick_data->set_instrument_id(p_d->ticker);
  tick_data->set_last_price(Max2zero(p_d->last_price));
  tick_data->set_bid_price1(Max2zero(p_d->bid[0]));
  tick_data->set_bid_volume1(p_d->bid_qty[0]);
  tick_data->set_ask_price1(Max2zero(p_d->ask[0]));
  tick_data->set_ask_volume1(p_d->ask_qty[0]);
  if (kDataLevel_ == 2) {
    tick_data->set_bid_price2(Max2zero(p_d->bid[1]));
    tick_data->set_bid_volume2(p_d->bid_qty[1]);
    tick_data->set_ask_price2(Max2zero(p_d->ask[1]));
    tick_data->set_ask_volume2(p_d->ask_qty[1]);
    tick_data->set_bid_price3(Max2zero(p_d->bid[2]));
    tick_data->set_bid_volume3(p_d->bid_qty[2]);
    tick_data->set_ask_price3(Max2zero(p_d->ask[2]));
    tick_data->set_ask_volume3(p_d->ask_qty[2]);
    tick_data->set_bid_price4(Max2zero(p_d->bid[3]));
    tick_data->set_bid_volume4(p_d->bid_qty[3]);
    tick_data->set_ask_price4(Max2zero(p_d->ask[3]));
    tick_data->set_ask_volume4(p_d->ask_qty[3]);
    tick_data->set_bid_price5(Max2zero(p_d->bid[4]));
    tick_data->set_bid_volume5(p_d->bid_qty[4]);
    tick_data->set_ask_price5(Max2zero(p_d->ask[4]));
    tick_data->set_ask_volume5(p_d->ask_qty[4]);
  }
  tick_data->set_open_price(Max2zero(p_d->open_price));
  tick_data->set_volume(p_d->qty);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "TickData." + tick_data->instrument_id();
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(DirectSender).SendMsg(msg);
}

void PublishData::DirectForwardDataToStrategy(BtpMarketDataStruct *p_d) {
  auto &market_ser = MarketService::GetInstance();
  auto pos = market_ser.ROLE(PublishControl).publish_para_map.find(p_d->instrument_id);
  if (pos != market_ser.ROLE(PublishControl).publish_para_map.end() && market_ser.login_state == kLoginState) {
    OnceFromDataflow(pos->second, p_d);
  } else if (pos == market_ser.ROLE(PublishControl).publish_para_map.end()) {
    ERROR_LOG("can not find ins from control para: %s", p_d->instrument_id);
  }
}

void PublishData::OnceFromDataflow(const PublishPara &p_c, BtpMarketDataStruct *p_d) {
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  tick_data->set_time_point(p_d->date_time);

  tick_data->set_state(strategy_market::TickData_TickState_active);
  tick_data->set_instrument_id(p_d->instrument_id);
  tick_data->set_last_price(Max2zero(p_d->last_price));
  tick_data->set_bid_price1(Max2zero(p_d->bid_price[0]));
  tick_data->set_bid_volume1(p_d->bid_volume[0]);
  tick_data->set_ask_price1(Max2zero(p_d->ask_price[0]));
  tick_data->set_ask_volume1(p_d->ask_volume[0]);
  if (kDataLevel_ == 2) {
    tick_data->set_bid_price2(Max2zero(p_d->bid_price[1]));
    tick_data->set_bid_volume2(p_d->bid_volume[1]);
    tick_data->set_ask_price2(Max2zero(p_d->ask_price[1]));
    tick_data->set_ask_volume2(p_d->ask_volume[1]);
    tick_data->set_bid_price3(Max2zero(p_d->bid_price[2]));
    tick_data->set_bid_volume3(p_d->bid_volume[2]);
    tick_data->set_ask_price3(Max2zero(p_d->ask_price[2]));
    tick_data->set_ask_volume3(p_d->ask_volume[2]);
    tick_data->set_bid_price4(Max2zero(p_d->bid_price[3]));
    tick_data->set_bid_volume4(p_d->bid_volume[3]);
    tick_data->set_ask_price4(Max2zero(p_d->ask_price[3]));
    tick_data->set_ask_volume4(p_d->ask_volume[3]);
    tick_data->set_bid_price5(Max2zero(p_d->bid_price[4]));
    tick_data->set_bid_volume5(p_d->bid_volume[4]);
    tick_data->set_ask_price5(Max2zero(p_d->ask_price[4]));
    tick_data->set_ask_volume5(p_d->ask_volume[4]);
  }
  // tick_data->set_open_price(Max2zero(p_d->open_price));
  tick_data->set_volume(p_d->volume);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "TickData." + tick_data->instrument_id();
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(DirectSender).SendMsg(msg);

  p_c.heartbeat = 0;
}

void PublishData::OnceFromDefault(const PublishPara &p_c, FtpMarketDataStruct *p_d) {
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  tick_data->set_time_point(p_d->date_time);

  tick_data->set_state(strategy_market::TickData_TickState_inactive);
  tick_data->set_instrument_id(p_d->instrument_id);
  tick_data->set_last_price(Max2zero(0.0));
  tick_data->set_bid_price1(Max2zero(0.0));
  tick_data->set_bid_volume1(0);
  tick_data->set_ask_price1(Max2zero(0.0));
  tick_data->set_ask_volume1(0);
  if (kDataLevel_ == 2) {
    tick_data->set_bid_price2(Max2zero(0.0));
    tick_data->set_bid_volume2(0);
    tick_data->set_ask_price2(Max2zero(0.0));
    tick_data->set_ask_volume2(0);
    tick_data->set_bid_price3(Max2zero(0.0));
    tick_data->set_bid_volume3(0);
    tick_data->set_ask_price3(Max2zero(0.0));
    tick_data->set_ask_volume3(0);
    tick_data->set_bid_price4(Max2zero(0.0));
    tick_data->set_bid_volume4(0);
    tick_data->set_ask_price4(Max2zero(0.0));
    tick_data->set_ask_volume4(0);
    tick_data->set_bid_price5(Max2zero(0.0));
    tick_data->set_bid_volume5(0);
    tick_data->set_ask_price5(Max2zero(0.0));
    tick_data->set_ask_volume5(0);
    tick_data->set_turnover(0);
    tick_data->set_open_interest(0);
    tick_data->set_upper_limit_price(Max2zero(0.0));
    tick_data->set_lower_limit_price(Max2zero(0.0));
    tick_data->set_pre_settlement_price(Max2zero(0.0));
    tick_data->set_pre_close_price(Max2zero(0.0));
    tick_data->set_pre_open_interest(0);
  }
  tick_data->set_open_price(Max2zero(0.0));

  tick_data->set_volume(0);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "TickData";
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(DirectSender).SendMsg(msg);

  p_c.heartbeat = 0;
}

void PublishData::DirectForwardDataToStrategy(MdsMktDataSnapshotT *p_d) {
  PZone("DirectForwardDataToStrategy");
  char instrument_id[16];
  sprintf(instrument_id, "%06d", p_d->head.instrId);
  auto &market_ser = MarketService::GetInstance();
  auto pos = market_ser.ROLE(PublishControl).publish_para_map.find(instrument_id);
  if (pos != market_ser.ROLE(PublishControl).publish_para_map.end() && market_ser.login_state == kLoginState) {
    OnceFromDataflow(pos->second, p_d);
  } else if (pos == market_ser.ROLE(PublishControl).publish_para_map.end()) {
    ERROR_LOG("can not find ins from control para: %06d", p_d->head.instrId);
  }
}

void PublishData::OnceFromDataflow(const PublishPara &p_c, MdsMktDataSnapshotT *p_d) {
  if (IsValidTickData(p_d) == false) {
    return;
  }

  char time_array[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  GetAssemblingTime(time_array, p_d);
  tick_data->set_time_point(time_array);

  tick_data->set_state(strategy_market::TickData_TickState_active);
  char instrument_id[16];
  sprintf(instrument_id, "%06d", p_d->head.instrId);
  tick_data->set_last_price(Max2zero(p_d->l2Stock.TradePx * 0.0001));
  tick_data->set_bid_price1(Max2zero(p_d->l2Stock.BidLevels[0].Price * 0.0001));
  tick_data->set_bid_volume1(p_d->l2Stock.BidLevels[0].NumberOfOrders);
  tick_data->set_ask_price1(Max2zero(p_d->l2Stock.OfferLevels[0].Price * 0.0001));
  tick_data->set_ask_volume1(p_d->l2Stock.OfferLevels[0].NumberOfOrders);
  if (kDataLevel_ == 2) {
    tick_data->set_bid_price2(Max2zero(p_d->l2Stock.BidLevels[1].Price * 0.0001));
    tick_data->set_bid_volume2(p_d->l2Stock.BidLevels[1].NumberOfOrders);
    tick_data->set_ask_price2(Max2zero(p_d->l2Stock.OfferLevels[1].Price * 0.0001));
    tick_data->set_ask_volume2(p_d->l2Stock.OfferLevels[1].NumberOfOrders);
    tick_data->set_bid_price3(Max2zero(p_d->l2Stock.BidLevels[2].Price * 0.0001));
    tick_data->set_bid_volume3(p_d->l2Stock.BidLevels[2].NumberOfOrders);
    tick_data->set_ask_price3(Max2zero(p_d->l2Stock.OfferLevels[2].Price * 0.0001));
    tick_data->set_ask_volume3(p_d->l2Stock.OfferLevels[2].NumberOfOrders);
    tick_data->set_bid_price4(Max2zero(p_d->l2Stock.BidLevels[3].Price * 0.0001));
    tick_data->set_bid_volume4(p_d->l2Stock.BidLevels[3].NumberOfOrders);
    tick_data->set_ask_price4(Max2zero(p_d->l2Stock.OfferLevels[3].Price * 0.0001));
    tick_data->set_ask_volume4(p_d->l2Stock.OfferLevels[3].NumberOfOrders);
    tick_data->set_bid_price5(Max2zero(p_d->l2Stock.BidLevels[4].Price * 0.0001));
    tick_data->set_bid_volume5(p_d->l2Stock.BidLevels[4].NumberOfOrders);
    tick_data->set_ask_price5(Max2zero(p_d->l2Stock.OfferLevels[4].Price * 0.0001));
    tick_data->set_ask_volume5(p_d->l2Stock.OfferLevels[4].NumberOfOrders);
  }
  tick_data->set_open_price(Max2zero(p_d->l2Stock.OpenPx * 0.0001));
  tick_data->set_volume(p_d->l2Stock.TotalVolumeTraded);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "TickData." + tick_data->instrument_id();
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(DirectSender).SendMsg(msg);

  p_c.heartbeat = 0;
}

void PublishData::DirectForwardDataToStrategy(FtpMarketDataStruct *p_d) {
  auto &market_ser = MarketService::GetInstance();
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto pos = market_ser.ROLE(PublishControl).publish_para_map.find(p_d->instrument_id);
  if (pos != market_ser.ROLE(PublishControl).publish_para_map.end() && market_ser.login_state == kLoginState) {
    if (p_d->state == 0 && json_cfg.GetConfig("market", "TimingPush") == "push") {
      OnceFromDefault(pos->second, p_d);
    } else {
      OnceFromDataflow(pos->second, p_d);
    }
  } else if (pos == market_ser.ROLE(PublishControl).publish_para_map.end()) {
    ERROR_LOG("can not find ins from control para: %s", p_d->instrument_id);
  }
}

void PublishData::OnceFromDataflow(const PublishPara &p_c, FtpMarketDataStruct *p_d) {
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  tick_data->set_time_point(p_d->date_time);

  tick_data->set_state(strategy_market::TickData_TickState_active);
  tick_data->set_instrument_id(p_d->instrument_id);
  tick_data->set_last_price(Max2zero(p_d->last_price));
  tick_data->set_bid_price1(Max2zero(p_d->bid_price[0]));
  tick_data->set_bid_volume1(p_d->bid_volume[0]);
  tick_data->set_ask_price1(Max2zero(p_d->ask_price[0]));
  tick_data->set_ask_volume1(p_d->ask_volume[0]);
  if (kDataLevel_ == 2) {
    tick_data->set_bid_price2(Max2zero(p_d->bid_price[1]));
    tick_data->set_bid_volume2(p_d->bid_volume[1]);
    tick_data->set_ask_price2(Max2zero(p_d->ask_price[1]));
    tick_data->set_ask_volume2(p_d->ask_volume[1]);
    tick_data->set_bid_price3(Max2zero(p_d->bid_price[2]));
    tick_data->set_bid_volume3(p_d->bid_volume[2]);
    tick_data->set_ask_price3(Max2zero(p_d->ask_price[2]));
    tick_data->set_ask_volume3(p_d->ask_volume[2]);
    tick_data->set_bid_price4(Max2zero(p_d->bid_price[3]));
    tick_data->set_bid_volume4(p_d->bid_volume[3]);
    tick_data->set_ask_price4(Max2zero(p_d->ask_price[3]));
    tick_data->set_ask_volume4(p_d->ask_volume[3]);
    tick_data->set_bid_price5(Max2zero(p_d->bid_price[4]));
    tick_data->set_bid_volume5(p_d->bid_volume[4]);
    tick_data->set_ask_price5(Max2zero(p_d->ask_price[4]));
    tick_data->set_ask_volume5(p_d->ask_volume[4]);
  }
  // tick_data->set_open_price(Max2zero(p_d->open_price));
  tick_data->set_volume(p_d->volume);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "TickData." + tick_data->instrument_id();
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(DirectSender).SendMsg(msg);

  p_c.heartbeat = 0;
}
