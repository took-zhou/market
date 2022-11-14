#include <fstream>
#include <iomanip>

#include "common/extern/json/fifo_map.hpp"
#include "common/extern/json/json.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"
#include "market/domain/components/control_para.h"

using namespace nlohmann;
template <class K, class V, class dummy_compare, class A>
using FifoWorkaroundFifoMap = fifo_map<K, V, fifo_map_compare<K>, A>;
using FifoJson = basic_json<FifoWorkaroundFifoMap>;

ControlPara::ControlPara() {
  auto &json_cfg = utils::JsonConfig::GetInstance();

  std::string temp_folder = json_cfg.GetConfig("market", "ControlParaFilePath").get<std::string>();
  json_path_ = temp_folder + "/controlPara/control.json";
  if (!utils::IsFileExist(json_path_)) {
    utils::CreatFile(json_path_);
  } else if (utils::GetFileSize(json_path_) > 0) {
    LoadFromJson();
  }
}

std::set<std::string> ControlPara::GetPridList(void) { return publish_paras_; }

bool ControlPara::InsertControlPara(const std::string &prid) {
  publish_paras_.emplace(prid);

  INFO_LOG("insert prid: %s.", prid.c_str());
  WriteToJson();
  return true;
}

bool ControlPara::EraseControlPara(const std::string &prid) {
  bool ret = false;

  if (publish_paras_.find(prid) == publish_paras_.end()) {
    ret = false;
    ERROR_LOG("can't find the prid: %s", prid.c_str());
  } else {
    ret = true;
    publish_paras_.erase(prid);
  }

  INFO_LOG("erase prid: %s.", prid.c_str());
  WriteToJson();
  return ret;
}

bool ControlPara::LoadFromJson(void) {
  int ret = true;
  FifoJson read_data;
  ifstream out_file(json_path_, ios::binary);
  if (out_file.is_open()) {
    out_file >> read_data;
    for (auto iter = read_data.begin(); iter != read_data.end(); iter++) {
      for (int i = 0; i < read_data[iter.key()].size(); i++) {
        std::string temp_prid;
        read_data[iter.key()][i].get_to(temp_prid);
        InsertControlPara(temp_prid);
      }
    }
  } else {
    WARNING_LOG("file:%s not exist.", json_path_.c_str());
    ret = false;
  }
  out_file.close();

  return ret;
}

bool ControlPara::WriteToJson(void) {
  int ret = true;
  FifoJson write_data;

  for (auto &item_pc : publish_paras_) {
    write_data["prid"].push_back(item_pc);
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