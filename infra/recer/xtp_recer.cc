#include "market/infra/recer/xtp_recer.h"
#include <stdio.h>
#include <iostream>
#include <memory>
#include "common/extern/log/log.h"
#include "common/self/protobuf/ipc.pb.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/infra/inner_zmq.h"

void XtpQuoteSpi::OnError(XTPRI *error_info) { IsErrorRspInfo(error_info); }

XtpQuoteSpi::XtpQuoteSpi() {}

XtpQuoteSpi::~XtpQuoteSpi() {}

void XtpQuoteSpi::OnDisconnected(int reason) {
  ERROR_LOG("front_disconnected, ErrorCode:%#x", reason);
  front_disconnected = true;
}

// market端ctp登入没有反馈，主动调用反馈接口
void XtpQuoteSpi::OnRspUserLogin(void) {
  std::unique_ptr<XTPRI> field(new XTPRI);
  field->error_id = 0;
  strcpy(field->error_msg, "force login");

  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(field.get()));
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "xtp_market";
  msg.msg_name = "OnRspUserLogin";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);

  front_disconnected = false;
}

// market端xtp登出没有反馈，主动调用反馈接口
void XtpQuoteSpi::OnRspUserLogout(void) {
  std::unique_ptr<XTPRI> field(new XTPRI);
  field->error_id = 0;
  strcpy(field->error_msg, "force logout");

  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(field.get()));
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "xtp_market";
  msg.msg_name = "OnRspUserLogout";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
}

void XtpQuoteSpi::OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[],
                                    int32_t ask1_count, int32_t max_ask1_count) {
#ifdef BENCH_TEST
  ScopedTimer t("OnDepthMarketData");
#endif
  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(market_data));
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "xtp_market";
  msg.msg_name = "OnDepthMarketData";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
}

void XtpQuoteSpi::OnQueryAllTickers(XTPQSI *ticker_info, XTPRI *error_info, bool is_last) {
  ipc::message req_msg;
  auto send_msg = req_msg.mutable_itp_msg();
  send_msg->set_address(reinterpret_cast<int64_t>(ticker_info));
  send_msg->set_is_last(is_last);
  utils::ItpMsg msg;
  req_msg.SerializeToString(&msg.pb_msg);
  msg.session_name = "xtp_market";
  msg.msg_name = "OnQueryAllTickers";

  auto &global_sem = GlobalSem::GetInstance();
  auto &inner_zmq = InnerZmq::GetInstance();
  inner_zmq.PushTask(msg);
  global_sem.WaitSemBySemName(GlobalSem::kApiRecv);
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

bool XtpQuoteSpi::IsErrorRspInfo(XTPRI *p_rsp_info) {
  bool b_result = ((p_rsp_info) && (p_rsp_info->error_id != 0));
  if (b_result) {
    ERROR_LOG("ErrorID: %d, ErrorMsg: %s", p_rsp_info->error_id, p_rsp_info->error_msg);
  }

  return b_result;
}
