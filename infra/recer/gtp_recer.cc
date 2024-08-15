#include "market/infra/recer/gtp_recer.h"
#include <stdio.h>
#include <iostream>
#include <memory>
#include "common/extern/log/log.h"
#include "common/self/global_sem.h"
#include "common/self/profiler.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/utils.h"
#include "market/infra/recer_sender.h"

// market端ctp登入没有反馈，主动调用反馈接口
void GtpMarketSpi::OnRspUserLogin(const GtpLoginLogoutStruct *login_info) {
  if (login_info != nullptr) {
    ipc::message req_msg;
    auto send_msg = req_msg.mutable_itp_msg();
    send_msg->set_address(reinterpret_cast<int64_t>(login_info));
    utils::ItpMsg msg;
    req_msg.SerializeToString(&msg.pb_msg);
    msg.session_name = "gtp_market";
    msg.msg_name = "OnRspUserLogin";

    auto &global_sem = GlobalSem::GetInstance();
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(InnerSender).SendMsg(msg);
    global_sem.WaitSemBySemName(SemName::kApiRecv);
    front_disconnected_ = false;
  } else {
    ERROR_LOG("login_info is nullptr");
  }
}

// market端btp登出没有反馈，主动调用反馈接口
void GtpMarketSpi::OnRspUserLogout(const GtpLoginLogoutStruct *login_info) {
  if (login_info != nullptr) {
    ipc::message req_msg;
    auto send_msg = req_msg.mutable_itp_msg();
    send_msg->set_address(reinterpret_cast<int64_t>(login_info));
    utils::ItpMsg msg;
    req_msg.SerializeToString(&msg.pb_msg);
    msg.session_name = "gtp_market";
    msg.msg_name = "OnRspUserLogout";

    auto &global_sem = GlobalSem::GetInstance();
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(InnerSender).SendMsg(msg);
    global_sem.WaitSemBySemName(SemName::kApiRecv);
  } else {
    ERROR_LOG("login_info is nullptr");
  }
}

void GtpMarketSpi::OnDepthMarketData(const GtpMarketDataStruct *market_data) {
#ifdef BENCH_TEST
  ScopedTimer timer("OnDepthMarketData");
#endif
  PZone("b");
  if (market_data != nullptr) {
    ipc::message req_msg;
    auto send_msg = req_msg.mutable_itp_msg();
    send_msg->set_address(reinterpret_cast<int64_t>(market_data));
    utils::ItpMsg msg;
    req_msg.SerializeToString(&msg.pb_msg);
    msg.session_name = "gtp_market";
    msg.msg_name = "OnDepthMarketData";

    auto &global_sem = GlobalSem::GetInstance();
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(InnerSender).SendMsg(msg);
    global_sem.WaitSemBySemName(SemName::kApiRecv);
  } else {
    ERROR_LOG("market_data is nullptr");
  }
}

void GtpMarketSpi::OnRspAllInstrumentInfo(GtpInstrumentInfo *ticker_info) {
  if (ticker_info != nullptr) {
    ipc::message req_msg;
    auto send_msg = req_msg.mutable_itp_msg();
    send_msg->set_address(reinterpret_cast<int64_t>(ticker_info));
    utils::ItpMsg msg;
    req_msg.SerializeToString(&msg.pb_msg);
    msg.session_name = "gtp_market";
    msg.msg_name = "OnRspAllInstrumentInfo";

    auto &global_sem = GlobalSem::GetInstance();
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(InnerSender).SendMsg(msg);
    global_sem.WaitSemBySemName(SemName::kApiRecv);
  } else {
    ERROR_LOG("ticker_info is nullptr");
  }
}

void GtpMarketSpi::OnFrontDisconnected(int reason) { front_disconnected_ = true; }

bool GtpMarketSpi::GetFrontDisconnected(void) { return front_disconnected_; }
