/*
 * ctpRecer.cpp
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#include "market/infra/recer/ctpRecer.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/semaphorePart.h"
#include "common/self/utils.h"
#include "market/infra/innerZmq.h"

void CtpMarketSpi::OnFrontConnected() {
  static int reConnect = 0;
  INFO_LOG("OnFrontConnected():is excuted...");
  // 在登出后系统会重新调用OnFrontConnected，这里简单判断并忽略第1次之后的所有调用。
  if (reConnect++ == 0) {
    auto &globalSem = GlobalSem::getInstance();
    globalSem.postSemBySemName(GlobalSem::loginLogout);
  }
}

void CtpMarketSpi::OnFrontDisconnected(int nReason) {
  ERROR_LOG("OnFrontDisconnected, ErrorCode:%#x", nReason);
  frontDisconnected = true;
}

void CtpMarketSpi::OnHeartBeatWarning(int nTimeLapse) { ERROR_LOG("OnHeartBeatWarning  %d!", nTimeLapse); }

void CtpMarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                  bool bIsLast) {
  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(pRspInfo));
  sendMsg->set_request_id(nRequestID);
  sendMsg->set_request_id(bIsLast);
  std::string reqStr;
  reqMsg.SerializeToString(&reqStr);

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.pushTask("ctp_market.OnRspUserLogin", reqStr.c_str());
  globalSem.waitSemBySemName(GlobalSem::apiRecv);
  frontDisconnected = false;

  globalSem.postSemBySemName(GlobalSem::loginLogout);
}

void CtpMarketSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(pRspInfo));
  sendMsg->set_request_id(nRequestID);
  sendMsg->set_request_id(bIsLast);
  std::string reqStr;
  reqMsg.SerializeToString(&reqStr);

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.pushTask("ctp_market.OnRspUserLogout", reqStr.c_str());
  globalSem.waitSemBySemName(GlobalSem::apiRecv);

  globalSem.postSemBySemName(GlobalSem::loginLogout);
}

void CtpMarketSpi::OnRspUserLogout(void) {
  CThostFtdcRspInfoField field;

  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(&field));
  std::string reqStr;
  reqMsg.SerializeToString(&reqStr);

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.pushTask("ctp_market.OnRspUserLogout", reqStr.c_str());
  globalSem.waitSemBySemName(GlobalSem::apiRecv);
}

void CtpMarketSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
#ifdef BENCH_TEST
  ScopedTimer t("OnRtnDepthMarketData");
#endif
  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(pDepthMarketData));
  std::string reqStr;
  reqMsg.SerializeToString(&reqStr);

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.pushTask("ctp_market.OnRtnDepthMarketData", reqStr.c_str());
  globalSem.waitSemBySemName(GlobalSem::apiRecv);
}

void CtpMarketSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) {}

void CtpMarketSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {}

void CtpMarketSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                                      int nRequestID, bool bIsLast) {}

void CtpMarketSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                                        int nRequestID, bool bIsLast) {}

void CtpMarketSpi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                                       int nRequestID, bool bIsLast) {}

void CtpMarketSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo,
                                         int nRequestID, bool bIsLast) {}

bool CtpRecer::receMsg(utils::ItpMsg &msg) {
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