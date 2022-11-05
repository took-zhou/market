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
    for (auto iter = read_data.begin(); iter != read_data.end(); iter++) {
      BacktestPara temp_control;
      read_data[iter.key()].at("indication").get_to(temp_control.indication);
      read_data[iter.key()].at("begin").get_to(temp_control.begin);
      read_data[iter.key()].at("end").get_to(temp_control.end);
      read_data[iter.key()].at("now").get_to(temp_control.now);
      read_data[iter.key()].at("speed").get_to(temp_control.speed);

      INFO_LOG("load prid: %s.", iter.key().c_str());
      backtest_para_map.insert(make_pair(iter.key(), temp_control));
    }
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

  for (auto &item_pc : backtest_para_map) {
    FifoJson one_item;
    one_item["indication"] = item_pc.second.indication;
    one_item["begin"] = item_pc.second.begin;
    one_item["end"] = item_pc.second.end;
    one_item["now"] = item_pc.second.now;
    one_item["speed"] = item_pc.second.speed;
    write_data[item_pc.first] = one_item;
  }

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

void BacktestControl::BuildControlPara(const std::string &keyname, const BacktestPara &para) {
  auto iter = backtest_para_map.find(keyname);
  if (iter != backtest_para_map.end()) {
    iter->second.begin = para.begin;
    iter->second.end = para.end;
    iter->second.speed = para.speed;
    iter->second.now = para.now;
  } else {
    backtest_para_map[keyname] = para;
  }
  INFO_LOG("insert prid: %s.", keyname.c_str());

  WriteToJson();
}

void BacktestControl::EraseControlPara(const std::string &keyname) {
  auto iter = backtest_para_map.find(keyname);
  if (iter != backtest_para_map.end()) {
    backtest_para_map.erase(iter);
    INFO_LOG("erase prid: %s.", keyname.c_str());
    WriteToJson();
  } else {
    INFO_LOG("not find prid: %s.", keyname.c_str());
  }
}

void BacktestControl::SetStartStopIndication(const std::string &keyname, ctpview_market::TickStartStopIndication_MessageType indication) {
  if (keyname == "") {
    for (auto &item : backtest_para_map) {
      item.second.indication = indication;
      INFO_LOG("prid: %s, setStartStopIndication %d.", item.first.c_str(), indication);
    }
    WriteToJson();
  } else {
    auto iter = backtest_para_map.find(keyname);
    if (iter != backtest_para_map.end()) {
      iter->second.indication = indication;
      INFO_LOG("prid: %s, setStartStopIndication %d.", keyname.c_str(), indication);
      WriteToJson();
    } else {
      INFO_LOG("not find prid: %s.", keyname.c_str());
    }
  }
}
