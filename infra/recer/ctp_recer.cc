/*
 * ctpRecer.cpp
 *
 *  Created on: 2020.11.30
 *      Author: Administrator
 */

#include "market/infra/recer/ctp_recer.h"
#include "common/extern/log/log.h"
#include "common/self/profiler.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/infra/inner_zmq.h"

void CtpMarketSpi::OnFrontConnected() {
  INFO_LOG("OnFrontConnected():is excuted...");
  // 在登出后系统会重新调用OnFrontConnected, 这里简单判断并忽略第1次之后的所有调用。
  if (re_connect++ == 0) {
    auto &global_sem = GlobalSem::GetInstance();
    global_sem.PostSemBySemName(GlobalSem::kLoginLogout);
  }
}

void CtpMarketSpi::OnFrontDisconnected(int reason) {
  ERROR_LOG("OnFrontDisconnected, ErrorCode:%#x", reason);
  front_disconnected = true;
}

void CtpMarketSpi::OnHeartBeatWarning(int n_time_lapse) { ERROR_LOG("OnHeartBeatWarning  %d!", n_time_lapse); }

void CtpMarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *p_rsp_user_login, CThostFtdcRspInfoField *p_rsp_info, int n_request_id,
                                  bool b_is_last) {
  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(p_rsp_info));
  send_msg->set_request_id(n_request_id);
  send_msg->set_request_id(b_is_last);
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "ctp_market";
  msg.msg_name = "OnRspUserLogin";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();

  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
  front_disconnected = false;
  global_sem.PostSemBySemName(GlobalSem::kLoginLogout);
}

void CtpMarketSpi::OnRspUserLogout(CThostFtdcUserLogoutField *user_logout, CThostFtdcRspInfoField *rsp_info, int n_request_id,
                                   bool b_is_last) {
  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(rsp_info));
  send_msg->set_request_id(n_request_id);
  send_msg->set_request_id(b_is_last);
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "ctp_market";
  msg.msg_name = "OnRspUserLogout";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
  global_sem.PostSemBySemName(GlobalSem::kLoginLogout);
}

void CtpMarketSpi::OnRspUserLogout(void) {
  CThostFtdcRspInfoField field;
  field.ErrorID = 0;
  strcpy(field.ErrorMsg, "force logout");

  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(&field));
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "ctp_market";
  msg.msg_name = "OnRspUserLogout";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
}

void CtpMarketSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *depth_market_data) {
#ifdef BENCH_TEST
  ScopedTimer t("OnRtnDepthMarketData");
#endif
  PZone("OnRtnDepthMarketData");
  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(depth_market_data));
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "ctp_market";
  msg.msg_name = "OnRtnDepthMarketData";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
}
