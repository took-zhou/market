/*
 * ctpRecer.cpp
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#include "market/infra/recer/ctp_recer.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/infra/inner_zmq.h"

void CtpMarketSpi::OnFrontConnected() {
  static int reConnect = 0;
  INFO_LOG("OnFrontConnected():is excuted...");
  // 在登出后系统会重新调用OnFrontConnected，这里简单判断并忽略第1次之后的所有调用。
  if (reConnect++ == 0) {
    auto &globalSem = GlobalSem::getInstance();
    globalSem.PostSemBySemName(GlobalSem::kLoginLogout);
  }
}

void CtpMarketSpi::OnFrontDisconnected(int nReason) {
  ERROR_LOG("OnFrontDisconnected, ErrorCode:%#x", nReason);
  front_disconnected = true;
}

void CtpMarketSpi::OnHeartBeatWarning(int nTimeLapse) { ERROR_LOG("OnHeartBeatWarning  %d!", nTimeLapse); }

void CtpMarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                  bool bIsLast) {
  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(pRspInfo));
  sendMsg->set_request_id(nRequestID);
  sendMsg->set_request_id(bIsLast);
  utils::ItpMsg msg;
  reqMsg.SerializeToString(&msg.pbMsg);
  msg.sessionName = "ctp_market";
  msg.msgName = "OnRspUserLogin";

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();

  innerZmq.PushTask(msg);
  globalSem.WaitSemBySemName(GlobalSem::kApiRecv);
  front_disconnected = false;
  globalSem.PostSemBySemName(GlobalSem::kLoginLogout);
}

void CtpMarketSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(pRspInfo));
  sendMsg->set_request_id(nRequestID);
  sendMsg->set_request_id(bIsLast);
  utils::ItpMsg msg;
  reqMsg.SerializeToString(&msg.pbMsg);
  msg.sessionName = "ctp_market";
  msg.msgName = "OnRspUserLogout";

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.PushTask(msg);
  globalSem.WaitSemBySemName(GlobalSem::kApiRecv);
  globalSem.PostSemBySemName(GlobalSem::kLoginLogout);
}

void CtpMarketSpi::OnRspUserLogout(void) {
  CThostFtdcRspInfoField field;

  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(&field));
  utils::ItpMsg msg;
  reqMsg.SerializeToString(&msg.pbMsg);
  msg.sessionName = "ctp_market";
  msg.msgName = "OnRspUserLogout";

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.PushTask(msg);
  globalSem.WaitSemBySemName(GlobalSem::kApiRecv);
}

void CtpMarketSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
#ifdef BENCH_TEST
  ScopedTimer t("OnRtnDepthMarketData");
#endif
  ipc::message reqMsg;
  auto sendMsg = reqMsg.mutable_itp_msg();
  sendMsg->set_address(reinterpret_cast<int64_t>(pDepthMarketData));
  utils::ItpMsg msg;
  reqMsg.SerializeToString(&msg.pbMsg);
  msg.sessionName = "ctp_market";
  msg.msgName = "OnRtnDepthMarketData";

  auto &globalSem = GlobalSem::getInstance();
  auto &innerZmq = InnerZmq::getInstance();
  innerZmq.PushTask(msg);
  globalSem.WaitSemBySemName(GlobalSem::kApiRecv);
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
