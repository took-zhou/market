// 实时时间获取
#include <stddef.h>
#include <time.h>

// 文件夹及文件操作相关
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <vector>

// 共享内存
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

//自定义头文件
#include "common/extern/log/log.h"
#include "common/self/fileUtil.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"
#include "market/domain/components/publishDepthMarketData.h"
#include "market/domain/marketService.h"
#include "market/infra/recer/ctpRecer.h"
#include "market/infra/recerSender.h"

constexpr U32 HEARTBEAT_WAIT_TIME = 60;

publishData::publishData() { ; }

void publishData::directForwardDataToStrategy(CThostFtdcDepthMarketDataField *pD) {
  auto &marketSer = MarketService::getInstance();
  auto pos = marketSer.ROLE(controlPara).publishCtrlMap.find(pD->InstrumentID);
  if (pos != marketSer.ROLE(controlPara).publishCtrlMap.end() && marketSer.login_state == LOGIN_STATE) {
    for (auto &item_pc : pos->second) {
      if (item_pc.indication == strategy_market::TickStartStopIndication_MessageType_start) {
        once_from_dataflow(item_pc, pD);
      }
    }
  }
}

void publishData::once_from_dataflow(const publishControl &pc, CThostFtdcDepthMarketDataField *pD) {
  if (pc.source == "rawtick") {
    once_from_dataflow_select_rawtick(pc, pD);
  } else if (pc.source == "level1") {
    once_from_dataflow_select_rawtick(pc, pD);
  }
}

void publishData::once_from_dataflow_select_rawtick(const publishControl &pc, CThostFtdcDepthMarketDataField *pD) {
  if (isValidTickData(pD) == false) {
    return;
  }

  char timeArray[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  getAssemblingTime(timeArray, pD);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(strategy_market::TickData_TickState_active);
  iter->set_instrument_id(pD->InstrumentID);
  iter->set_last_price(std::to_string(max2zero(pD->LastPrice)));
  iter->set_bid_price1(std::to_string(max2zero(pD->BidPrice1)));
  iter->set_bid_volume1(pD->BidVolume1);
  iter->set_ask_price1(std::to_string(max2zero(pD->AskPrice1)));
  iter->set_ask_volume1(pD->AskVolume1);
  if (data_level == 2) {
    iter->set_bid_price2(std::to_string(max2zero(pD->BidPrice2)));
    iter->set_bid_volume2(pD->BidVolume2);
    iter->set_ask_price2(std::to_string(max2zero(pD->AskPrice2)));
    iter->set_ask_volume2(pD->AskVolume2);
    iter->set_bid_price3(std::to_string(max2zero(pD->BidPrice3)));
    iter->set_bid_volume3(pD->BidVolume3);
    iter->set_ask_price3(std::to_string(max2zero(pD->AskPrice3)));
    iter->set_ask_volume3(pD->AskVolume3);
    iter->set_bid_price4(std::to_string(max2zero(pD->BidPrice4)));
    iter->set_bid_volume4(pD->BidVolume4);
    iter->set_ask_price4(std::to_string(max2zero(pD->AskPrice4)));
    iter->set_ask_volume4(pD->AskVolume4);
    iter->set_bid_price5(std::to_string(max2zero(pD->BidPrice5)));
    iter->set_bid_volume5(pD->BidVolume5);
    iter->set_ask_price5(std::to_string(max2zero(pD->AskPrice5)));
    iter->set_ask_volume5(pD->AskVolume5);
  }
  iter->set_open_price(std::to_string(max2zero(pD->OpenPrice)));
  iter->set_volume(pD->Volume);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pbMsg);
  msg.sessionName = "strategy_market";
  msg.msgName = "TickData." + pc.identify;
  auto &recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).send(msg);

  pc.heartbeat = 0;
}

void publishData::once_from_dataflow_select_level1(const publishControl &pc, CThostFtdcDepthMarketDataField *pD) {
  if (isValidTickData(pD) == false) {
    return;
  }

  if (isValidLevel1Data(pc, pD) == false) {
    return;
  }

  char timeArray[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  getAssemblingTime(timeArray, pD);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(strategy_market::TickData_TickState_active);
  iter->set_instrument_id(pD->InstrumentID);
  iter->set_last_price(std::to_string(max2zero(pD->LastPrice)));
  iter->set_bid_price1(std::to_string(max2zero(pD->BidPrice1)));
  iter->set_bid_volume1(pD->BidVolume1);
  iter->set_ask_price1(std::to_string(max2zero(pD->AskPrice1)));
  iter->set_ask_volume1(pD->AskVolume1);
  if (data_level == 2) {
    iter->set_bid_price2(std::to_string(max2zero(pD->BidPrice2)));
    iter->set_bid_volume2(pD->BidVolume2);
    iter->set_ask_price2(std::to_string(max2zero(pD->AskPrice2)));
    iter->set_ask_volume2(pD->AskVolume2);
    iter->set_bid_price3(std::to_string(max2zero(pD->BidPrice3)));
    iter->set_bid_volume3(pD->BidVolume3);
    iter->set_ask_price3(std::to_string(max2zero(pD->AskPrice3)));
    iter->set_ask_volume3(pD->AskVolume3);
    iter->set_bid_price4(std::to_string(max2zero(pD->BidPrice4)));
    iter->set_bid_volume4(pD->BidVolume4);
    iter->set_ask_price4(std::to_string(max2zero(pD->AskPrice4)));
    iter->set_ask_volume4(pD->AskVolume4);
    iter->set_bid_price5(std::to_string(max2zero(pD->BidPrice5)));
    iter->set_bid_volume5(pD->BidVolume5);
    iter->set_ask_price5(std::to_string(max2zero(pD->AskPrice5)));
    iter->set_ask_volume5(pD->AskVolume5);
  }

  iter->set_open_price(std::to_string(max2zero(pD->OpenPrice)));
  iter->set_volume(pD->Volume);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pbMsg);
  msg.sessionName = "strategy_market";
  msg.msgName = "TickData." + pc.identify;
  auto &recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).send(msg);

  pc.heartbeat = 0;
}

void publishData::heartbeatDetect() {
  auto &marketSer = MarketService::getInstance();
  while (1) {
    for (auto &item_pc : marketSer.ROLE(controlPara).publishCtrlMap) {
      for (auto &item_id : item_pc.second) {
        if (marketSer.login_state == LOGIN_STATE && item_id.indication == strategy_market::TickStartStopIndication_MessageType_start) {
          item_id.heartbeat++;
          if (item_id.heartbeat >= HEARTBEAT_WAIT_TIME) {
            once_from_default(item_id, item_pc.first);
          }
        }
      }
    }

    sleep(1);
  }
}

void publishData::once_from_default(const publishControl &pc, const string &keyname) {
  char timeArray[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  getLocalTime(timeArray);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(strategy_market::TickData_TickState_inactive);
  iter->set_instrument_id(keyname);
  iter->set_last_price(std::to_string(max2zero(0.0)));
  iter->set_bid_price1(std::to_string(max2zero(0.0)));
  iter->set_bid_volume1(0);
  iter->set_ask_price1(std::to_string(max2zero(0.0)));
  iter->set_ask_volume1(0);
  if (data_level == 2) {
    iter->set_bid_price2(std::to_string(max2zero(0.0)));
    iter->set_bid_volume2(0);
    iter->set_ask_price2(std::to_string(max2zero(0.0)));
    iter->set_ask_volume2(0);
    iter->set_bid_price3(std::to_string(max2zero(0.0)));
    iter->set_bid_volume3(0);
    iter->set_ask_price3(std::to_string(max2zero(0.0)));
    iter->set_ask_volume3(0);
    iter->set_bid_price4(std::to_string(max2zero(0.0)));
    iter->set_bid_volume4(0);
    iter->set_ask_price4(std::to_string(max2zero(0.0)));
    iter->set_ask_volume4(0);
    iter->set_bid_price5(std::to_string(max2zero(0.0)));
    iter->set_bid_volume5(0);
    iter->set_ask_price5(std::to_string(max2zero(0.0)));
    iter->set_ask_volume5(0);
    iter->set_turnover(0);
    iter->set_open_interest(0);
    iter->set_upper_limit_price(std::to_string(max2zero(0.0)));
    iter->set_lower_limit_price(std::to_string(max2zero(0.0)));
    iter->set_pre_settlement_price(std::to_string(max2zero(0.0)));
    iter->set_pre_close_price(std::to_string(max2zero(0.0)));
    iter->set_pre_open_interest(0);
  }
  iter->set_open_price(std::to_string(max2zero(0.0)));

  iter->set_volume(0);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pbMsg);
  msg.sessionName = "strategy_market";
  msg.msgName = "TickData." + pc.identify;
  auto &recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).send(msg);

  pc.heartbeat = 0;
}

bool publishData::isValidLevel1Data(const publishControl &pc, CThostFtdcDepthMarketDataField *pD) {
  bool ret = false;

  float ticksize = max2zero(pD->AskPrice1) - max2zero(pD->BidPrice1);
  if (fabs(ticksize - pc.ticksize) < 1e-6 && fabs(ticksize - 0) > 1e-6) {
    ret = true;
  }

  return ret;
}

void publishData::directForwardDataToStrategy(XTPMD *pD) {
  auto &marketSer = MarketService::getInstance();
  auto pos = marketSer.ROLE(controlPara).publishCtrlMap.find(pD->ticker);
  if (pos != marketSer.ROLE(controlPara).publishCtrlMap.end() && marketSer.login_state == LOGIN_STATE) {
    for (auto &item_pc : pos->second) {
      if (item_pc.indication == strategy_market::TickStartStopIndication_MessageType_start) {
        once_from_dataflow(item_pc, pD);
      }
    }
  }
}

void publishData::once_from_dataflow(const publishControl &pc, XTPMD *pD) {
  if (pc.source == "rawtick") {
    once_from_dataflow_select_rawtick(pc, pD);
  } else if (pc.source == "level1") {
    once_from_dataflow_select_level1(pc, pD);
  }
}

void publishData::once_from_dataflow_select_rawtick(const publishControl &pc, XTPMD *pD) {
  if (isValidTickData(pD) == false) {
    return;
  }

  char timeArray[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  getAssemblingTime(timeArray, pD);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(strategy_market::TickData_TickState_active);
  iter->set_instrument_id(pD->ticker);
  iter->set_last_price(std::to_string(max2zero(pD->last_price)));
  iter->set_bid_price1(std::to_string(max2zero(pD->bid[0])));
  iter->set_bid_volume1(pD->bid_qty[0]);
  iter->set_ask_price1(std::to_string(max2zero(pD->ask[0])));
  iter->set_ask_volume1(pD->ask_qty[0]);
  if (data_level == 2) {
    iter->set_bid_price2(std::to_string(max2zero(pD->bid[1])));
    iter->set_bid_volume2(pD->bid_qty[1]);
    iter->set_ask_price2(std::to_string(max2zero(pD->ask[1])));
    iter->set_ask_volume2(pD->ask_qty[1]);
    iter->set_bid_price3(std::to_string(max2zero(pD->bid[2])));
    iter->set_bid_volume3(pD->bid_qty[2]);
    iter->set_ask_price3(std::to_string(max2zero(pD->ask[2])));
    iter->set_ask_volume3(pD->ask_qty[2]);
    iter->set_bid_price4(std::to_string(max2zero(pD->bid[3])));
    iter->set_bid_volume4(pD->bid_qty[3]);
    iter->set_ask_price4(std::to_string(max2zero(pD->ask[3])));
    iter->set_ask_volume4(pD->ask_qty[3]);
    iter->set_bid_price5(std::to_string(max2zero(pD->bid[4])));
    iter->set_bid_volume5(pD->bid_qty[4]);
    iter->set_ask_price5(std::to_string(max2zero(pD->ask[4])));
    iter->set_ask_volume5(pD->ask_qty[4]);
  }
  iter->set_open_price(std::to_string(max2zero(pD->open_price)));
  iter->set_volume(pD->qty);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pbMsg);
  msg.sessionName = "strategy_market";
  msg.msgName = "TickData." + pc.identify;
  auto &recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).send(msg);
}

// 无法获取最小变动单位，暂不实现该功能
void publishData::once_from_dataflow_select_level1(const publishControl &pc, XTPMD *pD) {
  if (isValidTickData(pD) == false) {
    return;
  }

  if (isValidLevel1Data(pc, pD) == false) {
    return;
  }

  char timeArray[100] = {0};
  strategy_market::message tick;
  auto tick_data = tick.mutable_tick_data();

  getAssemblingTime(timeArray, pD);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(strategy_market::TickData_TickState_active);
  iter->set_instrument_id(pD->ticker);
  iter->set_last_price(std::to_string(max2zero(pD->last_price)));
  iter->set_bid_price1(std::to_string(max2zero(pD->bid[0])));
  iter->set_bid_volume1(pD->bid_qty[0]);
  iter->set_ask_price1(std::to_string(max2zero(pD->ask[0])));
  iter->set_ask_volume1(pD->ask_qty[0]);
  if (data_level == 2) {
    iter->set_bid_price2(std::to_string(max2zero(pD->bid[1])));
    iter->set_bid_volume2(pD->bid_qty[1]);
    iter->set_ask_price2(std::to_string(max2zero(pD->ask[1])));
    iter->set_ask_volume2(pD->ask_qty[1]);
    iter->set_bid_price3(std::to_string(max2zero(pD->bid[2])));
    iter->set_bid_volume3(pD->bid_qty[2]);
    iter->set_ask_price3(std::to_string(max2zero(pD->ask[2])));
    iter->set_ask_volume3(pD->ask_qty[2]);
    iter->set_bid_price4(std::to_string(max2zero(pD->bid[3])));
    iter->set_bid_volume4(pD->bid_qty[3]);
    iter->set_ask_price4(std::to_string(max2zero(pD->ask[3])));
    iter->set_ask_volume4(pD->ask_qty[3]);
    iter->set_bid_price5(std::to_string(max2zero(pD->bid[4])));
    iter->set_bid_volume5(pD->bid_qty[4]);
    iter->set_ask_price5(std::to_string(max2zero(pD->ask[4])));
    iter->set_ask_volume5(pD->ask_qty[4]);
  }
  iter->set_open_price(std::to_string(max2zero(pD->open_price)));
  iter->set_volume(pD->qty);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pbMsg);
  msg.sessionName = "strategy_market";
  msg.msgName = "TickData." + pc.identify;
  auto &recerSender = RecerSender::getInstance();
  recerSender.ROLE(Sender).ROLE(ProxySender).send(msg);
}

bool publishData::isValidLevel1Data(const publishControl &pc, XTPMD *pD) {
  bool ret = false;

  float ticksize = max2zero(pD->ask[0]) - max2zero(pD->bid[0]);
  if (fabs(ticksize - pc.ticksize) < 1e-6 && fabs(ticksize - 0) > 1e-6) {
    ret = true;
  }

  return ret;
}
