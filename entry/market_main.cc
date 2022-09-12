/*
 * marketMain.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include <chrono>
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"
#include "market/domain/market_service.h"
#include "market/infra/base_zmq.h"
#include "market/infra/recer_sender.h"
#include "market/interface/market_event.h"

int main(int argc, char *agrv[]) {
  auto &jsonCfg = utils::JsonConfig::getInstance();

  std::string marketLogPath = jsonCfg.get_config("market", "LogPath").get<std::string>();
  utils::CreatFolder(marketLogPath);
  LOG_INIT(marketLogPath.c_str(), "marketlog", 6);
  INFO_LOG("markt log path is %s", marketLogPath.c_str());

  std::string compile_time = utils::get_compile_time();
  jsonCfg.WriteConfig("market", "CompileTime", compile_time);
  INFO_LOG("program last build at %s.", compile_time.c_str());

  auto &marketSer = MarketService::getInstance();
  INFO_LOG("marketSer.init ok");

  std::this_thread::sleep_for(3s);

  auto &marketEvent = MarketEvent::getInstance();
  INFO_LOG("marketEvent.init ok");
  marketEvent.Run();

  return 0;
}
