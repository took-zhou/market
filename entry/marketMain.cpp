/*
 * marketMain.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include <chrono>
#include <string>
#include "common/extern/log/log.h"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"
#include "market/domain/marketService.h"
#include "market/infra/recerSender.h"
#include "market/infra/zmqBase.h"
#include "market/interface/marketEvent.h"

int main(int argc, char *agrv[]) {
  auto &jsonCfg = utils::JsonConfig::getInstance();

  //开启log
  std::string marketLogPath = jsonCfg.getConfig("market", "LogPath").get<std::string>();
  utils::creatFolder(marketLogPath);
  LOG_INIT(marketLogPath.c_str(), "marketlog", 6);
  INFO_LOG("TRADE LOG PATH is %s", marketLogPath.c_str());

  // 打印版本信息
  std::string compile_time = utils::GetCompileTime();
  jsonCfg.writeConfig("market", "version", compile_time);
  INFO_LOG("program last build at %s.", compile_time.c_str());

  //初始化zmq模块
  auto &zmq = ZmqBase::getInstance();
  while (!zmq.init()) {
    ERROR_LOG("zmq init failed");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  INFO_LOG("zmq init ok");
  //消息接收\发送器初始化
  auto &recerSender = RecerSender::getInstance();
  while (!recerSender.init()) {
    ERROR_LOG("recerSender init failed");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  INFO_LOG("recerSender.init ok");
  //初始化ctpmarket
  auto &marketSer = MarketService::getInstance();
  while (not marketSer.init()) {
    ERROR_LOG("marketSer init failed");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  INFO_LOG("marketSer.init ok");
  // 初始化strategyEvent模块
  auto &marketEvent = MarketEvent::getInstance();
  while (!marketEvent.init()) {
    ERROR_LOG("marketEvent init failed");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  INFO_LOG("marketEvent.init ok");
  // strategyEvent 循环等待消息触发
  marketEvent.run();
  return 0;
}
