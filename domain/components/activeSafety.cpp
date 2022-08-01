#include <unistd.h>

#include "common/extern/log/log.h"
#include "common/self/utils.h"
#include "market/domain/components/activeSafety.h"
#include "market/domain/marketService.h"
#include "market/infra/recerSender.h"

#include "common/self/semaphorePart.h"
extern GlobalSem globalSem;

activeSafety::activeSafety() {
  ;
  ;
}

void activeSafety::checkSafety() {
  time_t now = {0};
  struct tm *timenow = NULL;
  static int check_flag = false;

  while (1) {
    time(&now);
    timenow = localtime(&now);  //获取当前时间

    // 固定在下午6点开始检测
    if (timenow->tm_hour == 18 && timenow->tm_min == 0 && check_flag == false) {
      req_alive();
      check_flag = true;
    } else if (timenow->tm_hour == 18 && timenow->tm_min > 0 && check_flag == true) {
      check_flag = false;
    }

    sleep(1);
  }
}

void activeSafety::req_alive() {
  INFO_LOG("is going to check target is alive.");

  auto &marketSer = MarketService::getInstance();
  auto keyNameList = marketSer.ROLE(controlPara).getIdentifyList();
  for (auto &keyname : keyNameList) {
    strategy_market::message msg;
    auto active_safety = msg.mutable_active_req();

    strategy_market::ActiveSafetyReq_MessageType check_id = strategy_market::ActiveSafetyReq_MessageType_isrun;
    active_safety->set_safe_id(check_id);
    active_safety->set_process_random_id(keyname);

    std::string reqStr;
    msg.SerializeToString(&reqStr);
    auto &recerSender = RecerSender::getInstance();
    string topic = "strategy_market.ActiveSafetyReq." + keyname;
    recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), reqStr.c_str());

    std::string semName = "req_alive";
    globalSem.addOrderSem(semName);
    if (globalSem.waitSemBySemName(semName, 3) != 0) {
      req_alive_timeout(keyname);
    }
    globalSem.delOrderSem(semName);
    sleep(1);
  }

  INFO_LOG("check target alive has finished.");
}

void activeSafety::req_alive_timeout(const string &keyname) {
  auto &marketSer = MarketService::getInstance();
  marketSer.ROLE(controlPara).eraseControlPara(keyname);
}