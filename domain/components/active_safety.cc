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
  time_t now = {0};
  struct tm *timenow = NULL;
  static int check_flag = false;

  while (1) {
    time(&now);
    timenow = localtime(&now);  //获取当前时间

    // 固定在下午6点开始检测
    if (timenow->tm_hour == 18 && timenow->tm_min == 0 && check_flag == false) {
      ReqAlive();
      check_flag = true;
    } else if (timenow->tm_hour == 18 && timenow->tm_min > 0 && check_flag == true) {
      check_flag = false;
    }

    std::this_thread::sleep_for(1s);
  }
}

void ActiveSafety::ReqAlive() {
  INFO_LOG("is going to check target is alive.");

  auto &marketSer = MarketService::getInstance();
  auto keyNameList = marketSer.ROLE(ControlPara).get_prid_list();
  for (auto &keyname : keyNameList) {
    strategy_market::message reqMsg;
    auto active_safety = reqMsg.mutable_active_req();

    strategy_market::ActiveSafetyReq_MessageType check_id = strategy_market::ActiveSafetyReq_MessageType_isrun;
    active_safety->set_safe_id(check_id);
    active_safety->set_process_random_id(keyname);
    utils::ItpMsg msg;
    reqMsg.SerializeToString(&msg.pbMsg);
    msg.sessionName = "strategy_market";
    msg.msgName = "ActiveSafetyReq." + keyname;
    auto &recerSender = RecerSender::getInstance();
    recerSender.ROLE(Sender).ROLE(ProxySender).Send(msg);

    auto &globalSem = GlobalSem::getInstance();
    if (globalSem.WaitSemBySemName(GlobalSem::kViewDebug, 3) != 0) {
      ReqAliveTimeout(keyname);
    }
    std::this_thread::sleep_for(1s);
  }

  INFO_LOG("check target alive has finished.");
}

void ActiveSafety::ReqAliveTimeout(const string &keyname) {
  auto &marketSer = MarketService::getInstance();
  marketSer.ROLE(ControlPara).EraseControlPara(keyname);
}