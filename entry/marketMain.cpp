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

  std::string marketLogPath = jsonCfg.getConfig("market", "LogPath").get<std::string>();
  utils::creatFolder(marketLogPath);
  LOG_INIT(marketLogPath.c_str(), "marketlog", 6);
  INFO_LOG("markt log path is %s", marketLogPath.c_str());

  std::string compile_time = utils::GetCompileTime();
  jsonCfg.writeConfig("market", "CompileTime", compile_time);
  INFO_LOG("program last build at %s.", compile_time.c_str());

  auto &marketSer = MarketService::getInstance();
  INFO_LOG("marketSer.init ok");

  auto &marketEvent = MarketEvent::getInstance();
  INFO_LOG("marketEvent.init ok");
  marketEvent.run();

  return 0;
}
