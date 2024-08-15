/*
 * marketMain.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/entry/market_main.h"
#include <signal.h>
#include <chrono>
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/extern/pybind11/include/pybind11/embed.h"
#include "common/self/file_util.h"
#include "common/self/profiler.h"
#include "common/self/utils.h"
#include "market/domain/components/fd_manage.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"
#include "market/interface/market_event.h"


void SignalHandler(int signal) {
  auto &market_main = MarketMain::GetInstance();
  market_main.Exit();
}

void MarketMain::Entry(int argc, char *argv[]) {
  pybind11::scoped_interpreter python;
  pybind11::gil_scoped_release release;

  signal(SIGINT, SignalHandler);

  auto &json_cfg = utils::JsonConfig::GetInstance();
  json_cfg.SetFileName("/etc/marktrade/config.json");

  auto market_log_path = json_cfg.GetConfig("market", "LogPath").get<std::string>();
  utils::CreatFolder(market_log_path);
  LOG_INIT(market_log_path.c_str(), "marketlog", 6);
  INFO_LOG("markt log path is %s", market_log_path.c_str());

  auto market_data_path = json_cfg.GetConfig("market", "ControlParaFilePath").get<std::string>();
  utils::CreatFolder(market_data_path);
  profiler::FlameGraphWriter::Instance().SetFilePath(market_data_path);

  FdManage::GetInstance();
  RecerSender::GetInstance();

  auto &market_ser = MarketService::GetInstance();
  INFO_LOG("market_ser init ok");
  market_ser.Run();

  std::this_thread::sleep_for(std::chrono::seconds(3));

  auto &market_event = MarketEvent::GetInstance();
  INFO_LOG("market_event init ok");
  market_event.Run();

  HoldOn();
}

void MarketMain::HoldOn(void) {
  while (is_hold_on_) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void MarketMain::Exit(void) {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  is_hold_on_ = false;
}

const std::string &MarketMain::GetMarketName() { return market_name_; }

int main(int argc, char *argv[]) {
  auto &market_main = MarketMain::GetInstance();
  market_main.Entry(argc, argv);
  return 0;
}
