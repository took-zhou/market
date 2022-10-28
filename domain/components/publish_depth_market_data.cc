#include <thread>

//自定义头文件
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"
#include "market/domain/components/publish_depth_market_data.h"
#include "market/domain/market_service.h"
#include "market/infra/recer/ctp_recer.h"
#include "market/infra/recer_sender.h"

PublishData::PublishData() { ; }

void PublishData::DirectForwardDataToStrategy(CThostFtdcDepthMarketDataField *p_d) {
  auto &market_ser = MarketService::GetInstance();
  auto pos = market_ser.ROLE(ControlPara).publish_ctrl_map.find(p_d->InstrumentID);
  if (pos != market_ser.ROLE(ControlPara).publish_ctrl_map.end() && market_ser.login_state == kLoginState) {
    for (auto &item_p_c : pos->second) {
      if (item_p_c.indication == strategy_market::TickStartStopIndication_MessageType_start) {
        OnceFromDataflow(item_p_c, p_d);
      }
    }
  } else if (pos == market_ser.ROLE(ControlPara).publish_ctrl_map.end()) {
    ERROR_LOG("can not find ins from control para: %s", p_d->InstrumentID);
  }
}

void PublishData::OnceFromDataflow(const PublishControl &p_c, CThostFtdcDepthMarketDataField *p_d) {
  if (p_c.source == "rawtick") {
    OnceFromDataflowSelectRawtick(p_c, p_d);
  } else if (p_c.source == "level1") {
    OnceFromDataflowSelectLevel1(p_c, p_d);
  }
}

void PublishData::OnceFromDataflowSelectRawtick(const PublishControl &p_c, CThostFtdcDepthMarketDataField *p_d) {
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
  msg.msg_name = "TickData." + p_c.prid;
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);

  p_c.heartbeat = 0;
}

void PublishData::OnceFromDataflowSelectLevel1(const PublishControl &p_c, CThostFtdcDepthMarketDataField *p_d) {
  if (IsValidTickData(p_d) == false) {
    return;
  }

  if (IsValidLevel1Data(p_c, p_d) == false) {
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
  msg.msg_name = "TickData." + p_c.prid;
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);

  p_c.heartbeat = 0;
}

void PublishData::HeartBeatDetect() {
  auto &market_ser = MarketService::GetInstance();
  while (1) {
    for (auto &item_p_c : market_ser.ROLE(ControlPara).publish_ctrl_map) {
      for (auto &item_id : item_p_c.second) {
        if (market_ser.login_state == kLoginState && item_id.indication == strategy_market::TickStartStopIndication_MessageType_start) {
          item_id.heartbeat++;
          if (item_id.heartbeat >= kHeartBeatWaitTime_) {
            OnceFromDefault(item_id, item_p_c.first);
          }
        }
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void PublishData::OnceFromDefault(const PublishControl &p_c, const string &keyname) {
  char time_array[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  GetLocalTime(time_array);
  tick_data->set_time_point(time_array);

  tick_data->set_state(strategy_market::TickData_TickState_inactive);
  tick_data->set_instrument_id(keyname);
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
  msg.msg_name = "TickData." + p_c.prid;
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);

  p_c.heartbeat = 0;
}

bool PublishData::IsValidLevel1Data(const PublishControl &p_c, CThostFtdcDepthMarketDataField *p_d) {
  bool ret = false;

  auto &market_ser = MarketService::GetInstance();
  double ticksize_from_instrument_info = market_ser.ROLE(InstrumentInfo).GetTickSize(p_d->InstrumentID);
  double ticksize_from_rawtick = Max2zero(p_d->AskPrice1) - Max2zero(p_d->BidPrice1);

  if (fabs(ticksize_from_rawtick - ticksize_from_instrument_info) < 1e-6 && fabs(ticksize_from_rawtick - 0) > 1e-6) {
    ret = true;
  }

  return ret;
}

void PublishData::DirectForwardDataToStrategy(XTPMD *p_d) {
  auto &market_ser = MarketService::GetInstance();
  auto pos = market_ser.ROLE(ControlPara).publish_ctrl_map.find(p_d->ticker);
  if (pos != market_ser.ROLE(ControlPara).publish_ctrl_map.end() && market_ser.login_state == kLoginState) {
    for (auto &item_p_c : pos->second) {
      if (item_p_c.indication == strategy_market::TickStartStopIndication_MessageType_start) {
        OnceFromDataflow(item_p_c, p_d);
      }
    }
  } else if (pos == market_ser.ROLE(ControlPara).publish_ctrl_map.end()) {
    ERROR_LOG("can not find ins from control para: %s", p_d->ticker);
  }
}

void PublishData::OnceFromDataflow(const PublishControl &p_c, XTPMD *p_d) {
  if (p_c.source == "rawtick") {
    OnceFromDataflowSelectRawtick(p_c, p_d);
  } else if (p_c.source == "level1") {
    OnceFromDataflowSelectLevel1(p_c, p_d);
  }
}

void PublishData::OnceFromDataflowSelectRawtick(const PublishControl &p_c, XTPMD *p_d) {
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
  msg.msg_name = "TickData." + p_c.prid;
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);
}

// 无法获取最小变动单位，暂不实现该功能
void PublishData::OnceFromDataflowSelectLevel1(const PublishControl &p_c, XTPMD *p_d) {
  if (IsValidTickData(p_d) == false) {
    return;
  }

  if (IsValidLevel1Data(p_c, p_d) == false) {
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
  msg.msg_name = "TickData." + p_c.prid;
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);
}

bool PublishData::IsValidLevel1Data(const PublishControl &p_c, XTPMD *p_d) {
  bool ret = false;

  auto &market_ser = MarketService::GetInstance();
  double ticksize_from_instrument_info = market_ser.ROLE(InstrumentInfo).GetTickSize(p_d->ticker);
  double ticksize_from_rawtick = Max2zero(p_d->ask[0]) - Max2zero(p_d->bid[0]);

  if (fabs(ticksize_from_rawtick - ticksize_from_instrument_info) < 1e-6 && fabs(ticksize_from_rawtick - 0) > 1e-6) {
    ret = true;
  }

  return ret;
}

void PublishData::DirectForwardDataToStrategy(BtpMarketDataStruct *p_d) {
  auto &market_ser = MarketService::GetInstance();
  auto pos = market_ser.ROLE(ControlPara).publish_ctrl_map.find(p_d->instrument_id);
  if (pos != market_ser.ROLE(ControlPara).publish_ctrl_map.end() && market_ser.login_state == kLoginState) {
    for (auto &item_p_c : pos->second) {
      if (item_p_c.indication == strategy_market::TickStartStopIndication_MessageType_start && stoi(item_p_c.prid) == p_d->prid) {
        OnceFromDataflow(item_p_c, p_d);
      }
    }
  } else if (pos == market_ser.ROLE(ControlPara).publish_ctrl_map.end()) {
    ERROR_LOG("can not find ins from control para: %s", p_d->instrument_id);
  }
}
void PublishData::OnceFromDataflow(const PublishControl &p_c, BtpMarketDataStruct *p_d) {
  if (p_c.source == "rawtick") {
    OnceFromDataflowSelectRawtick(p_c, p_d);
  } else if (p_c.source == "level1") {
    OnceFromDataflowSelectLevel1(p_c, p_d);
  }
}

void PublishData::OnceFromDataflowSelectRawtick(const PublishControl &p_c, BtpMarketDataStruct *p_d) {
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  tick_data->set_time_point(p_d->date_time);

  if (p_d->state == 1) {
    tick_data->set_state(strategy_market::TickData_TickState_active);
  } else {
    tick_data->set_state(strategy_market::TickData_TickState_inactive);
  }

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
  msg.msg_name = "TickData." + p_c.prid;
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);
}
void PublishData::OnceFromDataflowSelectLevel1(const PublishControl &p_c, BtpMarketDataStruct *p_d) {
  if (IsValidLevel1Data(p_c, p_d) == false) {
    return;
  }

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
  msg.msg_name = "TickData." + p_c.prid;
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);
}

bool PublishData::IsValidLevel1Data(const PublishControl &p_c, BtpMarketDataStruct *p_d) {
  bool ret = false;

  auto &market_ser = MarketService::GetInstance();
  double ticksize_from_instrument_info = market_ser.ROLE(InstrumentInfo).GetTickSize(p_d->instrument_id);
  double ticksize_from_rawtick = Max2zero(p_d->ask_price[0]) - Max2zero(p_d->bid_price[0]);

  if (fabs(ticksize_from_rawtick - ticksize_from_instrument_info) < 1e-6 && fabs(ticksize_from_rawtick - 0) > 1e-6) {
    ret = true;
  }

  return ret;
}
