#include <fstream>
#include <iomanip>

#include "market/domain/components/backtest_control.h"

#include "common/extern/json/fifo_map.hpp"
#include "common/extern/json/json.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"

using namespace nlohmann;
template <class K, class V, class dummy_compare, class A>
using FifoWorkaroundFifoMap = fifo_map<K, V, fifo_map_compare<K>, A>;
using FifoJson = basic_json<FifoWorkaroundFifoMap>;

BacktestControl::BacktestControl(void) {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto users = json_cfg.GetConfig("market", "User");
  for (auto &user : users) {
    std::string temp_folder = json_cfg.GetConfig("market", "ControlParaFilePath").get<std::string>();
    json_path_ = temp_folder + "/" + (std::string)user + "/BacktestControl/control.json";
    if (!utils::IsFileExist(json_path_)) {
      utils::CreatFile(json_path_);
    } else if (utils::GetFileSize(json_path_) > 0) {
      LoadFromJson();
    }
    break;
  }
}

bool BacktestControl::LoadFromJson(void) {
  int ret = true;
  FifoJson read_data;
  ifstream out_file(json_path_, ios::binary);
  if (out_file.is_open()) {
    out_file >> read_data;
    read_data.at("indication").get_to(backtest_para_.indication);
    read_data.at("begin").get_to(backtest_para_.begin);
    read_data.at("end").get_to(backtest_para_.end);
    read_data.at("now").get_to(backtest_para_.now);
    read_data.at("speed").get_to(backtest_para_.speed);
  } else {
    WARNING_LOG("file:%s not exist.", json_path_.c_str());
    ret = false;
  }
  out_file.close();

  return ret;
}

bool BacktestControl::WriteToJson(void) {
  int ret = true;
  FifoJson write_data;
  write_data["indication"] = backtest_para_.indication;
  write_data["begin"] = backtest_para_.begin;
  write_data["end"] = backtest_para_.end;
  write_data["now"] = backtest_para_.now;
  write_data["speed"] = backtest_para_.speed;
  ofstream in_file(json_path_);
  if (in_file.is_open()) {
    in_file << setw(4) << write_data << endl;
  } else {
    ERROR_LOG("file:%s open error.", json_path_.c_str());
    ret = false;
  }
  in_file.close();

  return ret;
}

void BacktestControl::BuildControlPara(const BacktestPara &para) {
  backtest_para_ = para;
  WriteToJson();
}

void BacktestControl::SetStartStopIndication(ctpview_market::TickStartStopIndication_MessageType indication) {
  backtest_para_.indication = indication;
  WriteToJson();
}
