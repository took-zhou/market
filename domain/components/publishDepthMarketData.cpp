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
#include "common/self/protobuf/market-strategy.pb.h"
#include "common/self/utils.h"
#include "market/domain/components/publishDepthMarketData.h"
#include "market/domain/marketService.h"
#include "market/infra/recer/ctpRecer.h"
#include "market/infra/recerSender.h"

constexpr U32 HEARTBEAT_WAIT_TIME = 60;

publishData::publishData() { ; }

void publishData::directForwardDataToStrategy(CThostFtdcDepthMarketDataField *pD) {
  auto &marketSer = MarketService::getInstance();
  utils::InstrumtntID tempIns;
  tempIns.ins = pD->InstrumentID;
  std::map<std::string, publishControl>::iterator mapit = marketSer.ROLE(controlPara).publishCtrlMap.begin();
  while (mapit != marketSer.ROLE(controlPara).publishCtrlMap.end()) {
    auto pos = mapit->second.instrumentList.find(tempIns);
    if (pos != end(mapit->second.instrumentList) && mapit->second.directforward == true) {
      if (mapit->second.indication == market_strategy::TickStartStopIndication_MessageType_start) {
        auto &marketSer = MarketService::getInstance();
        if (marketSer.ROLE(Market).ROLE(CtpMarketApi).getMarketLoginState() == LOGIN_STATE) {
          once_from_dataflow(mapit, pD);
        }
      }
    }
    mapit++;
  }
}

void publishData::once_from_dataflow(std::map<std::string, publishControl>::iterator pc, CThostFtdcDepthMarketDataField *pD) {
  if (pc->second.source == "rawtick") {
    once_from_dataflow_select_rawtick(pc, pD);
  } else if (pc->second.source == "level1") {
    once_from_dataflow_select_rawtick(pc, pD);
  }
}

void publishData::once_from_dataflow_select_rawtick(std::map<std::string, publishControl>::iterator pc,
                                                    CThostFtdcDepthMarketDataField *pD) {
  if (isValidTickData(pD) == false) {
    return;
  }

  char timeArray[100] = {0};
  market_strategy::message tick;
  auto tick_data = tick.mutable_tick_data();

  getAssemblingTime(timeArray, pD);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(market_strategy::TickData_TickState_active);
  iter->set_instrument_id(pD->InstrumentID);
  iter->set_last_price(std::to_string(max2zero(pD->LastPrice)));
  iter->set_bid_price1(std::to_string(max2zero(pD->BidPrice1)));
  iter->set_bid_volume1(pD->BidVolume1);
  iter->set_ask_price1(std::to_string(max2zero(pD->AskPrice1)));
  iter->set_ask_volume1(pD->AskVolume1);
  // iter->set_bid_price2(std::to_string(max2zero(pD->BidPrice2)));
  // iter->set_bid_volume2(pD->BidVolume2);
  // iter->set_ask_price2(std::to_string(max2zero(pD->AskPrice2)));
  // iter->set_ask_volume2(pD->AskVolume2);
  // iter->set_bid_price3(std::to_string(max2zero(pD->BidPrice3)));
  // iter->set_bid_volume3(pD->BidVolume3);
  // iter->set_ask_price3(std::to_string(max2zero(pD->AskPrice3)));
  // iter->set_ask_volume3(pD->AskVolume3);
  // iter->set_bid_price4(std::to_string(max2zero(pD->BidPrice4)));
  // iter->set_bid_volume4(pD->BidVolume4);
  // iter->set_ask_price4(std::to_string(max2zero(pD->AskPrice4)));
  // iter->set_ask_volume4(pD->AskVolume4);
  // iter->set_bid_price5(std::to_string(max2zero(pD->BidPrice5)));
  // iter->set_bid_volume5(pD->BidVolume5);
  // iter->set_ask_price5(std::to_string(max2zero(pD->AskPrice5)));
  // iter->set_ask_volume5(pD->AskVolume5);
  iter->set_open_price(std::to_string(max2zero(pD->OpenPrice)));
  iter->set_volume(pD->Volume);

  std::string tickStr;
  tick.SerializeToString(&tickStr);

  auto &recerSender = RecerSender::getInstance();
  string topic = "market_strategy.TickData." + pc->first;
  recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());

  pc->second.heartbeat = 0;
}

// 无法获取最小变动单位，暂不实现该功能
void publishData::once_from_dataflow_select_level1(std::map<std::string, publishControl>::iterator pc, CThostFtdcDepthMarketDataField *pD) {
  if (isValidTickData(pD) == false) {
    return;
  }

  if (isValidLevel1Data(pc, pD) == false) {
    return;
  }

  char timeArray[100] = {0};
  market_strategy::message tick;
  auto tick_data = tick.mutable_tick_data();

  getAssemblingTime(timeArray, pD);
  tick_data->set_time_point(timeArray);

  auto iter = tick_data->add_tick_list();
  iter->set_state(market_strategy::TickData_TickState_active);
  iter->set_instrument_id(pD->InstrumentID);
  iter->set_last_price(std::to_string(max2zero(pD->LastPrice)));
  iter->set_bid_price1(std::to_string(max2zero(pD->BidPrice1)));
  iter->set_bid_volume1(pD->BidVolume1);
  iter->set_ask_price1(std::to_string(max2zero(pD->AskPrice1)));
  iter->set_ask_volume1(pD->AskVolume1);
  // iter->set_bid_price2(std::to_string(max2zero(pD->BidPrice2)));
  // iter->set_bid_volume2(pD->BidVolume2);
  // iter->set_ask_price2(std::to_string(max2zero(pD->AskPrice2)));
  // iter->set_ask_volume2(pD->AskVolume2);
  // iter->set_bid_price3(std::to_string(max2zero(pD->BidPrice3)));
  // iter->set_bid_volume3(pD->BidVolume3);
  // iter->set_ask_price3(std::to_string(max2zero(pD->AskPrice3)));
  // iter->set_ask_volume3(pD->AskVolume3);
  // iter->set_bid_price4(std::to_string(max2zero(pD->BidPrice4)));
  // iter->set_bid_volume4(pD->BidVolume4);
  // iter->set_ask_price4(std::to_string(max2zero(pD->AskPrice4)));
  // iter->set_ask_volume4(pD->AskVolume4);
  // iter->set_bid_price5(std::to_string(max2zero(pD->BidPrice5)));
  // iter->set_bid_volume5(pD->BidVolume5);
  // iter->set_ask_price5(std::to_string(max2zero(pD->AskPrice5)));
  // iter->set_ask_volume5(pD->AskVolume5);
  iter->set_open_price(std::to_string(max2zero(pD->OpenPrice)));
  iter->set_volume(pD->Volume);

  std::string tickStr;
  tick.SerializeToString(&tickStr);

  auto &recerSender = RecerSender::getInstance();
  string topic = "market_strategy.TickData." + pc->first;
  recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());

  pc->second.heartbeat = 0;
}

void publishData::heartbeatDetect() {
  auto &marketSer = MarketService::getInstance();
  std::map<string, publishControl>::iterator iter;
  while (1) {
    for (iter = marketSer.ROLE(controlPara).publishCtrlMap.begin(); iter != marketSer.ROLE(controlPara).publishCtrlMap.end(); iter++) {
      if (iter->second.indication == market_strategy::TickStartStopIndication_MessageType_start) {
        if (marketSer.ROLE(Market).ROLE(CtpMarketApi).getMarketLoginState() == LOGIN_STATE) {
          iter->second.heartbeat++;
          if (iter->second.heartbeat >= HEARTBEAT_WAIT_TIME) {
            // INFO_LOG("%s heartbeat wait times out, data will be transferred
            // default data", iter->first.c_str());
            once_from_default(iter);
          }
        }
      }
    }

    sleep(1);
  }
}

void publishData::once_from_default(std::map<std::string, publishControl>::iterator pc) {
  char timeArray[100] = {0};
  market_strategy::message tick;
  auto tick_data = tick.mutable_tick_data();

  getLocalTime(timeArray);
  tick_data->set_time_point(timeArray);

  auto ins_iter = pc->second.instrumentList.begin();
  while (ins_iter != pc->second.instrumentList.end()) {
    auto iter = tick_data->add_tick_list();
    iter->set_state(market_strategy::TickData_TickState_inactive);
    iter->set_instrument_id(ins_iter->ins);
    iter->set_last_price(std::to_string(max2zero(0.0)));
    iter->set_bid_price1(std::to_string(max2zero(0.0)));
    iter->set_bid_volume1(0);
    iter->set_ask_price1(std::to_string(max2zero(0.0)));
    iter->set_ask_volume1(0);
    // iter->set_bid_price2(std::to_string(max2zero(tickData->datafield[instrument_iter->index].BidPrice2)));
    // iter->set_bid_volume2(tickData->datafield[instrument_iter->index].BidVolume2);
    // iter->set_ask_price2(std::to_string(max2zero(tickData->datafield[instrument_iter->index].AskPrice2)));
    // iter->set_ask_volume2(tickData->datafield[instrument_iter->index].AskVolume2);
    // iter->set_bid_price3(std::to_string(max2zero(tickData->datafield[instrument_iter->index].BidPrice3)));
    // iter->set_bid_volume3(tickData->datafield[instrument_iter->index].BidVolume3);
    // iter->set_ask_price3(std::to_string(max2zero(tickData->datafield[instrument_iter->index].AskPrice3)));
    // iter->set_ask_volume3(tickData->datafield[instrument_iter->index].AskVolume3);
    // iter->set_bid_price4(std::to_string(max2zero(tickData->datafield[instrument_iter->index].BidPrice4)));
    // iter->set_bid_volume4(tickData->datafield[instrument_iter->index].BidVolume4);
    // iter->set_ask_price4(std::to_string(max2zero(tickData->datafield[instrument_iter->index].AskPrice4)));
    // iter->set_ask_volume4(tickData->datafield[instrument_iter->index].AskVolume4);
    // iter->set_bid_price5(std::to_string(max2zero(tickData->datafield[instrument_iter->index].BidPrice5)));
    // iter->set_bid_volume5(tickData->datafield[instrument_iter->index].BidVolume5);
    // iter->set_ask_price5(std::to_string(max2zero(tickData->datafield[instrument_iter->index].AskPrice5)));
    // iter->set_ask_volume5(tickData->datafield[instrument_iter->index].AskVolume5);
    // iter->set_turnover(tickData->datafield[instrument_iter->index].Turnover);
    // iter->set_open_interest(tickData->datafield[instrument_iter->index].OpenInterest);
    // iter->set_upper_limit_price(std::to_string(max2zero(tickData->datafield[instrument_iter->index].UpperLimitPrice)));
    // iter->set_lower_limit_price(std::to_string(max2zero(tickData->datafield[instrument_iter->index].LowerLimitPrice)));
    iter->set_open_price(std::to_string(max2zero(0.0)));
    // iter->set_pre_settlement_price(std::to_string(max2zero(tickData->datafield[instrument_iter->index].PreSettlementPrice)));
    // iter->set_pre_close_price(std::to_string(max2zero(tickData->datafield[instrument_iter->index].PreClosePrice)));
    // iter->set_pre_open_interest(tickData->datafield[instrument_iter->index].PreOpenInterest);
    iter->set_volume(0);

    ins_iter++;
  }

  std::string tickStr;
  tick.SerializeToString(&tickStr);

  auto &recerSender = RecerSender::getInstance();
  char temp_topic[100];
  sprintf(temp_topic, "market_strategy.TickData.%s", pc->first.c_str());
  recerSender.ROLE(Sender).ROLE(ProxySender).send(temp_topic, tickStr.c_str());

  pc->second.heartbeat = 0;
}

bool publishData::isValidLevel1Data(std::map<std::string, publishControl>::iterator pc, CThostFtdcDepthMarketDataField *pD) {
  bool ret = false;
  utils::InstrumtntID tempIns;
  tempIns.ins = pD->InstrumentID;

  auto pos = pc->second.instrumentList.find(tempIns);
  if (pos != end(pc->second.instrumentList)) {
    float ticksize = max2zero(pD->AskPrice1) - max2zero(pD->BidPrice1);
    if (fabs(ticksize - pos->ticksize) < 1e-6 && fabs(ticksize - 0) > 1e-6) {
      ret = true;
    }
  }

  return ret;
}
