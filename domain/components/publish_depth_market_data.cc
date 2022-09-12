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

void PublishData::DirectForwardDataToStrategy(CThostFtdcDepthMarketDataField *pD) {
  auto &marketSer = MarketService::getInstance();
  auto pos = marketSer.ROLE(ControlPara).publish_ctrl_map.find(pD->InstrumentID);
  if (pos != marketSer.ROLE(ControlPara).publish_ctrl_map.end() && marketSer.login_state == kLoginState) {
    for (auto &item_pc : pos->second) {
      if (item_pc.indication == strategy_market::TickStartStopIndication_MessageType_start) {
        OnceFromDataflow(item_pc, pD);
      }
    }
  }
}

void PublishData::OnceFromDataflow(const PublishControl &pc, CThostFtdcDepthMarketDataField *pD) {
  if (pc.source == "rawtick") {
    OnceFromDataflowSelectRawtick(pc, pD);
  } else if (pc.source == "level1") {
    OnceFromDataflowSelectLevel1(pc, pD);
  }
}

void PublishData::OnceFromDataflowSelectRawtick(const PublishControl &pc, CThostFtdcDepthMarketDataField *pD) {
  if (IsValidTickData(pD) == false) {
    return;
  }

  char timeArray[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  get_assembling_time(timeArray, pD);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(strategy_market::TickData_TickState_active);
  iter->set_instrument_id(pD->InstrumentID);
  iter->set_last_price(Max2zero(pD->LastPrice));
  iter->set_bid_price1(Max2zero(pD->BidPrice1));
  iter->set_bid_volume1(pD->BidVolume1);
  iter->set_ask_price1(Max2zero(pD->AskPrice1));
  iter->set_ask_volume1(pD->AskVolume1);
  if (kDataLevel == 2) {
    iter->set_bid_price2(Max2zero(pD->BidPrice2));
    iter->set_bid_volume2(pD->BidVolume2);
    iter->set_ask_price2(Max2zero(pD->AskPrice2));
    iter->set_ask_volume2(pD->AskVolume2);
    iter->set_bid_price3(Max2zero(pD->BidPrice3));
    iter->set_bid_volume3(pD->BidVolume3);
    iter->set_ask_price3(Max2zero(pD->AskPrice3));
    iter->set_ask_volume3(pD->AskVolume3);
    iter->set_bid_price4(Max2zero(pD->BidPrice4));
    iter->set_bid_volume4(pD->BidVolume4);
    iter->set_ask_price4(Max2zero(pD->AskPrice4));
    iter->set_ask_volume4(pD->AskVolume4);
    iter->set_bid_price5(Max2zero(pD->BidPrice5));
    iter->set_bid_volume5(pD->BidVolume5);
    iter->set_ask_price5(Max2zero(pD->AskPrice5));
    iter->set_ask_volume5(pD->AskVolume5);
  }
  iter->set_open_price(Max2zero(pD->OpenPrice));
  iter->set_volume(pD->Volume);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pbMsg);
  msg.sessionName = "strategy_market";
  msg.msgName = "TickData." + pc.prid;
  auto &recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).Send(msg);

  pc.heartbeat = 0;
}

void PublishData::OnceFromDataflowSelectLevel1(const PublishControl &pc, CThostFtdcDepthMarketDataField *pD) {
  if (IsValidTickData(pD) == false) {
    return;
  }

  if (IsValidLevel1Data(pc, pD) == false) {
    return;
  }

  char timeArray[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  get_assembling_time(timeArray, pD);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(strategy_market::TickData_TickState_active);
  iter->set_instrument_id(pD->InstrumentID);
  iter->set_last_price(Max2zero(pD->LastPrice));
  iter->set_bid_price1(Max2zero(pD->BidPrice1));
  iter->set_bid_volume1(pD->BidVolume1);
  iter->set_ask_price1(Max2zero(pD->AskPrice1));
  iter->set_ask_volume1(pD->AskVolume1);
  if (kDataLevel == 2) {
    iter->set_bid_price2(Max2zero(pD->BidPrice2));
    iter->set_bid_volume2(pD->BidVolume2);
    iter->set_ask_price2(Max2zero(pD->AskPrice2));
    iter->set_ask_volume2(pD->AskVolume2);
    iter->set_bid_price3(Max2zero(pD->BidPrice3));
    iter->set_bid_volume3(pD->BidVolume3);
    iter->set_ask_price3(Max2zero(pD->AskPrice3));
    iter->set_ask_volume3(pD->AskVolume3);
    iter->set_bid_price4(Max2zero(pD->BidPrice4));
    iter->set_bid_volume4(pD->BidVolume4);
    iter->set_ask_price4(Max2zero(pD->AskPrice4));
    iter->set_ask_volume4(pD->AskVolume4);
    iter->set_bid_price5(Max2zero(pD->BidPrice5));
    iter->set_bid_volume5(pD->BidVolume5);
    iter->set_ask_price5(Max2zero(pD->AskPrice5));
    iter->set_ask_volume5(pD->AskVolume5);
  }

  iter->set_open_price(Max2zero(pD->OpenPrice));
  iter->set_volume(pD->Volume);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pbMsg);
  msg.sessionName = "strategy_market";
  msg.msgName = "TickData." + pc.prid;
  auto &recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).Send(msg);

  pc.heartbeat = 0;
}

void PublishData::HeartBeatDetect() {
  auto &marketSer = MarketService::getInstance();
  while (1) {
    for (auto &item_pc : marketSer.ROLE(ControlPara).publish_ctrl_map) {
      for (auto &item_id : item_pc.second) {
        if (marketSer.login_state == kLoginState && item_id.indication == strategy_market::TickStartStopIndication_MessageType_start) {
          item_id.heartbeat++;
          if (item_id.heartbeat >= kHeartBeatWaitTime) {
            OnceFromDefault(item_id, item_pc.first);
          }
        }
      }
    }

    std::this_thread::sleep_for(1000ms);
  }
}

void PublishData::OnceFromDefault(const PublishControl &pc, const string &keyname) {
  char timeArray[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  get_local_time(timeArray);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(strategy_market::TickData_TickState_inactive);
  iter->set_instrument_id(keyname);
  iter->set_last_price(Max2zero(0.0));
  iter->set_bid_price1(Max2zero(0.0));
  iter->set_bid_volume1(0);
  iter->set_ask_price1(Max2zero(0.0));
  iter->set_ask_volume1(0);
  if (kDataLevel == 2) {
    iter->set_bid_price2(Max2zero(0.0));
    iter->set_bid_volume2(0);
    iter->set_ask_price2(Max2zero(0.0));
    iter->set_ask_volume2(0);
    iter->set_bid_price3(Max2zero(0.0));
    iter->set_bid_volume3(0);
    iter->set_ask_price3(Max2zero(0.0));
    iter->set_ask_volume3(0);
    iter->set_bid_price4(Max2zero(0.0));
    iter->set_bid_volume4(0);
    iter->set_ask_price4(Max2zero(0.0));
    iter->set_ask_volume4(0);
    iter->set_bid_price5(Max2zero(0.0));
    iter->set_bid_volume5(0);
    iter->set_ask_price5(Max2zero(0.0));
    iter->set_ask_volume5(0);
    iter->set_turnover(0);
    iter->set_open_interest(0);
    iter->set_upper_limit_price(Max2zero(0.0));
    iter->set_lower_limit_price(Max2zero(0.0));
    iter->set_pre_settlement_price(Max2zero(0.0));
    iter->set_pre_close_price(Max2zero(0.0));
    iter->set_pre_open_interest(0);
  }
  iter->set_open_price(Max2zero(0.0));

  iter->set_volume(0);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pbMsg);
  msg.sessionName = "strategy_market";
  msg.msgName = "TickData." + pc.prid;
  auto &recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).Send(msg);

  pc.heartbeat = 0;
}

bool PublishData::IsValidLevel1Data(const PublishControl &pc, CThostFtdcDepthMarketDataField *pD) {
  bool ret = false;

  float ticksize = Max2zero(pD->AskPrice1) - Max2zero(pD->BidPrice1);
  if (fabs(ticksize - pc.ticksize) < 1e-6 && fabs(ticksize - 0) > 1e-6) {
    ret = true;
  }

  return ret;
}

void PublishData::DirectForwardDataToStrategy(XTPMD *pD) {
  auto &marketSer = MarketService::getInstance();
  auto pos = marketSer.ROLE(ControlPara).publish_ctrl_map.find(pD->ticker);
  if (pos != marketSer.ROLE(ControlPara).publish_ctrl_map.end() && marketSer.login_state == kLoginState) {
    for (auto &item_pc : pos->second) {
      if (item_pc.indication == strategy_market::TickStartStopIndication_MessageType_start) {
        OnceFromDataflow(item_pc, pD);
      }
    }
  }
}

void PublishData::OnceFromDataflow(const PublishControl &pc, XTPMD *pD) {
  if (pc.source == "rawtick") {
    OnceFromDataflowSelectRawtick(pc, pD);
  } else if (pc.source == "level1") {
    OnceFromDataflowSelectLevel1(pc, pD);
  }
}

void PublishData::OnceFromDataflowSelectRawtick(const PublishControl &pc, XTPMD *pD) {
  if (IsValidTickData(pD) == false) {
    return;
  }

  char timeArray[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  get_assembling_time(timeArray, pD);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(strategy_market::TickData_TickState_active);
  iter->set_instrument_id(pD->ticker);
  iter->set_last_price(Max2zero(pD->last_price));
  iter->set_bid_price1(Max2zero(pD->bid[0]));
  iter->set_bid_volume1(pD->bid_qty[0]);
  iter->set_ask_price1(Max2zero(pD->ask[0]));
  iter->set_ask_volume1(pD->ask_qty[0]);
  if (kDataLevel == 2) {
    iter->set_bid_price2(Max2zero(pD->bid[1]));
    iter->set_bid_volume2(pD->bid_qty[1]);
    iter->set_ask_price2(Max2zero(pD->ask[1]));
    iter->set_ask_volume2(pD->ask_qty[1]);
    iter->set_bid_price3(Max2zero(pD->bid[2]));
    iter->set_bid_volume3(pD->bid_qty[2]);
    iter->set_ask_price3(Max2zero(pD->ask[2]));
    iter->set_ask_volume3(pD->ask_qty[2]);
    iter->set_bid_price4(Max2zero(pD->bid[3]));
    iter->set_bid_volume4(pD->bid_qty[3]);
    iter->set_ask_price4(Max2zero(pD->ask[3]));
    iter->set_ask_volume4(pD->ask_qty[3]);
    iter->set_bid_price5(Max2zero(pD->bid[4]));
    iter->set_bid_volume5(pD->bid_qty[4]);
    iter->set_ask_price5(Max2zero(pD->ask[4]));
    iter->set_ask_volume5(pD->ask_qty[4]);
  }
  iter->set_open_price(Max2zero(pD->open_price));
  iter->set_volume(pD->qty);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pbMsg);
  msg.sessionName = "strategy_market";
  msg.msgName = "TickData." + pc.prid;
  auto &recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).Send(msg);
}

// 无法获取最小变动单位，暂不实现该功能
void PublishData::OnceFromDataflowSelectLevel1(const PublishControl &pc, XTPMD *pD) {
  if (IsValidTickData(pD) == false) {
    return;
  }

  if (IsValidLevel1Data(pc, pD) == false) {
    return;
  }

  char timeArray[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  get_assembling_time(timeArray, pD);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(strategy_market::TickData_TickState_active);
  iter->set_instrument_id(pD->ticker);
  iter->set_last_price(Max2zero(pD->last_price));
  iter->set_bid_price1(Max2zero(pD->bid[0]));
  iter->set_bid_volume1(pD->bid_qty[0]);
  iter->set_ask_price1(Max2zero(pD->ask[0]));
  iter->set_ask_volume1(pD->ask_qty[0]);
  if (kDataLevel == 2) {
    iter->set_bid_price2(Max2zero(pD->bid[1]));
    iter->set_bid_volume2(pD->bid_qty[1]);
    iter->set_ask_price2(Max2zero(pD->ask[1]));
    iter->set_ask_volume2(pD->ask_qty[1]);
    iter->set_bid_price3(Max2zero(pD->bid[2]));
    iter->set_bid_volume3(pD->bid_qty[2]);
    iter->set_ask_price3(Max2zero(pD->ask[2]));
    iter->set_ask_volume3(pD->ask_qty[2]);
    iter->set_bid_price4(Max2zero(pD->bid[3]));
    iter->set_bid_volume4(pD->bid_qty[3]);
    iter->set_ask_price4(Max2zero(pD->ask[3]));
    iter->set_ask_volume4(pD->ask_qty[3]);
    iter->set_bid_price5(Max2zero(pD->bid[4]));
    iter->set_bid_volume5(pD->bid_qty[4]);
    iter->set_ask_price5(Max2zero(pD->ask[4]));
    iter->set_ask_volume5(pD->ask_qty[4]);
  }
  iter->set_open_price(Max2zero(pD->open_price));
  iter->set_volume(pD->qty);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pbMsg);
  msg.sessionName = "strategy_market";
  msg.msgName = "TickData." + pc.prid;
  auto &recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).Send(msg);
}

bool PublishData::IsValidLevel1Data(const PublishControl &pc, XTPMD *pD) {
  bool ret = false;

  float ticksize = Max2zero(pD->ask[0]) - Max2zero(pD->bid[0]);
  if (fabs(ticksize - pc.ticksize) < 1e-6 && fabs(ticksize - 0) > 1e-6) {
    ret = true;
  }

  return ret;
}
