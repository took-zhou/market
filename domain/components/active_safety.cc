#include <unistd.h>
#include <thread>

#include "common/extern/log/log.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/domain/components/active_safety.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"

ActiveSafety::ActiveSafety() { ; }

void ActiveSafety::CheckSafety() {
  static int check_flag = false;

  auto &market_ser = MarketService::GetInstance();
  auto timenow = market_ser.ROLE(MarketTimeState).GetTimeNow();

  // 固定在下午6点开始检测
  if (timenow != nullptr && timenow->tm_hour == 18 && timenow->tm_min == 0 && check_flag == false) {
    ReqAlive();
    check_flag = true;
  } else if (timenow != nullptr && timenow->tm_hour == 18 && timenow->tm_min > 0 && check_flag == true) {
    check_flag = false;
  }
}

void ActiveSafety::ReqAlive() {
  INFO_LOG("is going to check target is alive.");

  auto &market_ser = MarketService::GetInstance();
  auto key_name_list = market_ser.ROLE(ControlPara).GetPridList();
  for (auto &keyname : key_name_list) {
    strategy_market::message req_msg;
    auto active_safety = req_msg.mutable_active_req();

    strategy_market::ActiveSafetyReq_MessageType check_id = strategy_market::ActiveSafetyReq_MessageType_isrun;
    active_safety->set_safe_id(check_id);
    active_safety->set_process_random_id(keyname);
    utils::ItpMsg msg;
    req_msg.SerializeToString(&msg.pb_msg);
    msg.session_name = "strategy_market";
    msg.msg_name = "ActiveSafetyReq." + keyname;
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(Sender).ROLE(ProxySender).SendMsg(msg);

    auto &global_sem = GlobalSem::GetInstance();
    if (global_sem.WaitSemBySemName(GlobalSem::kStrategyRsp, 3) != 0) {
      ReqAliveTimeout(keyname);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  INFO_LOG("check target alive has finished.");
}

void ActiveSafety::ReqAliveTimeout(const string &keyname) {
  auto &market_ser = MarketService::GetInstance();
  market_ser.ROLE(PublishControl).ErasePublishPara(keyname);
  market_ser.ROLE(ControlPara).EraseControlPara(keyname);
}