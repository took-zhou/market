#include <fstream>
#include <iomanip>

#include "market/domain/components/publish_control.h"

#include "common/extern/json/fifo_map.hpp"
#include "common/extern/json/json.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"

using namespace nlohmann;
template <class K, class V, class dummy_compare, class A>
using FifoWorkaroundFifoMap = fifo_map<K, V, fifo_map_compare<K>, A>;
using FifoJson = basic_json<FifoWorkaroundFifoMap>;

PublishControl::PublishControl(void) {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto users = json_cfg.GetConfig("market", "User");
  for (auto &user : users) {
    std::string temp_folder = json_cfg.GetConfig("market", "ControlParaFilePath").get<std::string>();
    json_path_ = temp_folder + "/" + (std::string)user + "/PublishControl/control.json";
    if (!utils::IsFileExist(json_path_)) {
      utils::CreatFile(json_path_);
    } else if (utils::GetFileSize(json_path_) > 0) {
      LoadFromJson();
    }
    break;
  }
}

bool PublishControl::LoadFromJson(void) {
  int ret = true;
  FifoJson read_data;
  ifstream out_file(json_path_, ios::binary);
  if (out_file.is_open()) {
    out_file >> read_data;
    for (auto iter = read_data.begin(); iter != read_data.end(); iter++) {
      PublishPara temp_control;
      read_data[iter.key()].at("exch").get_to(temp_control.exch);
      read_data[iter.key()].at("source").get_to(temp_control.source);

      INFO_LOG("load instrument: %s.", iter.key().c_str());
      publish_para_map.insert(make_pair(iter.key(), temp_control));
    }
  } else {
    WARNING_LOG("file:%s not exist.", json_path_.c_str());
    ret = false;
  }
  out_file.close();

  return ret;
}

bool PublishControl::WriteToJson(void) {
  int ret = true;
  FifoJson write_data;

  for (auto &item_pc : publish_para_map) {
    FifoJson one_item;
    one_item["exch"] = item_pc.second.exch;
    one_item["source"] = item_pc.second.source;
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

void PublishControl::BuildPublishPara(const std::string &ins, const PublishPara &para) {
  auto iter = publish_para_map.find(ins);
  if (iter == publish_para_map.end()) {
    publish_para_map[ins] = para;
    INFO_LOG("insert ins: %s.", ins.c_str());
    WriteToJson();
  }
}

void PublishControl::ErasePublishPara(const std::string &ins) {
  for (auto publish_iter = publish_para_map.begin(); publish_iter != publish_para_map.end();) {
    if (publish_iter->first == ins || ins == "") {
      INFO_LOG("ins: %s, doesn't exist anymore, erase it.", publish_iter->first.c_str());
      publish_iter = publish_para_map.erase(publish_iter);
    } else {
      publish_iter++;
    }
  }

  WriteToJson();
}

std::vector<utils::InstrumtntID> PublishControl::GetInstrumentList() {
  std::vector<utils::InstrumtntID> instrument_vec;
  instrument_vec.clear();

  for (auto &item_pc : publish_para_map) {
    utils::InstrumtntID item_ins;
    item_ins.ins = item_pc.first;
    item_ins.exch = item_pc.second.exch;
    instrument_vec.push_back(item_ins);
  }

  return instrument_vec;
}
