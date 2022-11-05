#include "market/infra/recer/btp_recer.h"
#include <stdio.h>
#include <iostream>
#include <memory>
#include "common/extern/log/log.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/infra/inner_zmq.h"

// market端ctp登入没有反馈，主动调用反馈接口
void BtpMarketSpi::OnRspUserLogin(const BtpLoginLogoutStruct *login_info) {
  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(login_info));
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "btp_market";
  msg.msg_name = "OnRspUserLogin";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
}

// market端btp登出没有反馈，主动调用反馈接口
void BtpMarketSpi::OnRspUserLogout(const BtpLoginLogoutStruct *login_info) {
  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(login_info));
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "btp_market";
  msg.msg_name = "OnRspUserLogout";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
}

void BtpMarketSpi::OnDepthMarketData(const BtpMarketDataStruct *market_data) {
#ifdef BENCH_TEST
  ScopedTimer timer("OnDepthMarketData");
#endif
  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(market_data));
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "btp_market";
  msg.msg_name = "OnDepthMarketData";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
}

void BtpMarketSpi::OnRspAllInstrumentInfo(BtpInstrumentInfo *ticker_info) {
  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(ticker_info));
  send_msg->set_request_id(ticker_info->prid);
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "btp_market";
  msg.msg_name = "OnRspAllInstrumentInfo";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
}