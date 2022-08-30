#include "market/infra/recer/xtpRecer.h"
#include <stdio.h>
#include <iostream>
#include <memory>
#include "common/extern/log/log.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/semaphorePart.h"
#include "common/self/utils.h"
#include "market/infra/innerZmq.h"

void XtpQuoteSpi::OnError(XTPRI *error_info, bool is_last) { IsErrorRspInfo(error_info); }

XtpQuoteSpi::XtpQuoteSpi() {}

XtpQuoteSpi::~XtpQuoteSpi() {}

void XtpQuoteSpi::OnDisconnected(int reason) {
  ERROR_LOG("OnFrontDisconnected, ErrorCode:%#x", reason);
  frontDisconnected = true;
}

// market端ctp登入没有反馈，主动调用反馈接口
void XtpQuoteSpi::OnRspUserLogin(void) {
  std::unique_ptr<XTPRI> field = std::make_unique<XTPRI>();
  field->error_id = 0;
  strcpy(field->error_msg, "force login");

  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(field.get()));
  std::string reqStr;
  reqMsg.SerializeToString(&reqStr);

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.pushTask("xtp_market.OnRspUserLogin", reqStr.c_str());
  globalSem.waitSemBySemName(GlobalSem::apiRecv);

  frontDisconnected = false;
}

// market端xtp登出没有反馈，主动调用反馈接口
void XtpQuoteSpi::OnRspUserLogout(void) {
  std::unique_ptr<XTPRI> field = std::make_unique<XTPRI>();
  field->error_id = 0;
  strcpy(field->error_msg, "force logout");

  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(field.get()));
  std::string reqStr;
  reqMsg.SerializeToString(&reqStr);

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.pushTask("xtp_market.OnRspUserLogout", reqStr.c_str());
  globalSem.waitSemBySemName(GlobalSem::apiRecv);
}

void XtpQuoteSpi::OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[],
                                    int32_t ask1_count, int32_t max_ask1_count) {
  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(market_data));
  std::string reqStr;
  reqMsg.SerializeToString(&reqStr);

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.pushTask("xtp_market.OnDepthMarketData", reqStr.c_str());
  globalSem.waitSemBySemName(GlobalSem::apiRecv);
}

void XtpQuoteSpi::OnQueryAllTickers(XTPQSI *ticker_info, XTPRI *error_info, bool is_last) {
  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(ticker_info));
  sendMsg->set_is_last(is_last);
  std::string reqStr;
  reqMsg.SerializeToString(&reqStr);

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.pushTask("xtp_market.OnQueryAllTickers", reqStr.c_str());
  globalSem.waitSemBySemName(GlobalSem::apiRecv);
}

void XtpQuoteSpi::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last) {}

void XtpQuoteSpi::OnUnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last) {}

void XtpQuoteSpi::OnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last) {}

void XtpQuoteSpi::OnUnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last) {}

void XtpQuoteSpi::OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) {}

void XtpQuoteSpi::OnUnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) {}

void XtpQuoteSpi::OnOrderBook(XTPOB *order_book) {}

void XtpQuoteSpi::OnTickByTick(XTPTBT *tbt_data) {}

void XtpQuoteSpi::OnQueryTickersPriceInfo(XTPTPI *ticker_info, XTPRI *error_info, bool is_last) {}

void XtpQuoteSpi::OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnUnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnUnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnUnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnSubscribeAllOptionMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnUnSubscribeAllOptionMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnSubscribeAllOptionOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnUnSubscribeAllOptionOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnSubscribeAllOptionTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

void XtpQuoteSpi::OnUnSubscribeAllOptionTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {}

bool XtpQuoteSpi::IsErrorRspInfo(XTPRI *pRspInfo) {
  bool bResult = ((pRspInfo) && (pRspInfo->error_id != 0));
  if (bResult) {
    ERROR_LOG("ErrorID: %d, ErrorMsg: %s", pRspInfo->error_id, pRspInfo->error_msg);
  }

  return bResult;
}

bool XtpRecer::receMsg(utils::ItpMsg &msg) {
  bool out = true;
  auto &innerZmqBase = InnerZmq::getInstance();

  char *recContent = innerZmqBase.pullTask();
  if (recContent != nullptr) {
    int index = 0;
    int segIndex = 0;
    int length = strlen(recContent) + 1;
    char temp[length];
    for (int i = 0; i < length; i++) {
      temp[index] = recContent[i];
      if (recContent[i] == '.' && segIndex == 0) {
        temp[index] = '\0';
        msg.sessionName = temp;
        index = 0;
        segIndex++;
      } else if (recContent[i] == ' ' && segIndex == 1) {
        temp[index] = '\0';
        msg.msgName = temp;
        index = 0;
        segIndex++;
      } else if (recContent[i] == '\0' && segIndex == 2) {
        msg.pbMsg = temp;
        break;
      } else {
        index++;
      }
    }
  } else {
    out = false;
  }

  return out;
}
