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
  auto &json_cfg = utils::JsonConfig::GetInstance();

  std::string market_log_path = json_cfg.GetConfig("market", "LogPath").get<std::string>();
  utils::CreatFolder(market_log_path);
  LOG_INIT(market_log_path.c_str(), "marketlog", 6);
  INFO_LOG("markt log path is %s", market_log_path.c_str());

  std::string compile_time = utils::GetCompileTime();
  json_cfg.WriteConfig("market", "CompileTime", compile_time);
  INFO_LOG("program last build at %s.", compile_time.c_str());

  auto &market_ser = MarketService::GetInstance();
  INFO_LOG("market_ser init ok");

  std::this_thread::sleep_for(3s);

  auto &market_event = MarketEvent::GetInstance();
  INFO_LOG("market_event init ok");
  market_event.Run();

  return 0;
}
