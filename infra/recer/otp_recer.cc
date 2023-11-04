#include "market/infra/recer/otp_recer.h"
#include <stdio.h>
#include <iostream>
#include <memory>
#include "common/extern/log/log.h"
#include "common/self/profiler.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/infra/recer_sender.h"

// market端ctp登入没有反馈，主动调用反馈接口
void OtpMarketSpi::OnRspUserLogin(const std::string *login_info) {
  if (login_info != nullptr) {
    ipc::message req_msg;
    auto send_msg = req_msg.mutable_itp_msg();
    send_msg->set_address(reinterpret_cast<int64_t>(&login_info));
    utils::ItpMsg msg;
    req_msg.SerializeToString(&msg.pb_msg);
    msg.session_name = "otp_market";
    msg.msg_name = "OnRspUserLogin";

    auto &global_sem = GlobalSem::GetInstance();
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(InnerSender).SendMsg(msg);
    global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
  } else {
    ERROR_LOG("login_info is nullptr");
  }
}

// market端btp登出没有反馈，主动调用反馈接口
void OtpMarketSpi::OnRspUserLogout(const std::string *logout_info) {
  if (logout_info != nullptr) {
    ipc::message req_msg;
    auto send_msg = req_msg.mutable_itp_msg();
    send_msg->set_address(reinterpret_cast<int64_t>(logout_info));
    utils::ItpMsg msg;
    req_msg.SerializeToString(&msg.pb_msg);
    msg.session_name = "otp_market";
    msg.msg_name = "OnRspUserLogout";

    auto &global_sem = GlobalSem::GetInstance();
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(InnerSender).SendMsg(msg);
    global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
  } else {
    ERROR_LOG("logout_info is nullptr");
  }
}

void OtpMarketSpi::OnRspStockStaticInfo(const MdsStockStaticInfoT *static_info, bool is_last) {
  if (static_info != nullptr) {
    ipc::message req_msg;
    auto send_msg = req_msg.mutable_itp_msg();
    send_msg->set_address(reinterpret_cast<int64_t>(static_info));
    send_msg->set_is_last(is_last);
    utils::ItpMsg msg;
    req_msg.SerializeToString(&msg.pb_msg);
    msg.session_name = "otp_market";
    msg.msg_name = "OnRspStockStaticInfo";

    auto &global_sem = GlobalSem::GetInstance();
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(InnerSender).SendMsg(msg);
    global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
  } else {
    ERROR_LOG("static_info is nullptr");
  }
}

void OtpMarketSpi::OnDepthMarketData(const MdsMktDataSnapshotT *market_data) {
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
    msg.session_name = "otp_market";
    msg.msg_name = "OnDepthMarketData";

    auto &global_sem = GlobalSem::GetInstance();
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(InnerSender).SendMsg(msg);
    global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
  } else {
    ERROR_LOG("market_data is nullptr");
  }
}
