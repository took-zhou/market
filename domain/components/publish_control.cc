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
      std::vector<PublishPara> tempcontrol_vec;
      for (int i = 0; i < read_data[iter.key()].size(); i++) {
        PublishPara temp_control;
        read_data[iter.key()][i].at("exch").get_to(temp_control.exch);
        read_data[iter.key()][i].at("prid").get_to(temp_control.prid);
        read_data[iter.key()][i].at("source").get_to(temp_control.source);

        tempcontrol_vec.push_back(temp_control);
        INFO_LOG("load prid: %s, instrument: %s.", temp_control.prid.c_str(), iter.key().c_str());
      }
      publish_para_map.insert(make_pair(iter.key(), tempcontrol_vec));
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
    for (auto &item_id : item_pc.second) {
      FifoJson one_item;
      one_item["exch"] = item_id.exch;
      one_item["prid"] = item_id.prid;
      one_item["source"] = item_id.source;
      write_data[item_pc.first].push_back(one_item);
    }
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

void PublishControl::BuildPublishPara(const std::string &keyname, const PublishPara &para) {
  auto iter = publish_para_map.find(keyname);
  if (iter != publish_para_map.end()) {
    for (auto &item : iter->second) {
      if (item.prid == para.prid) {
        return;
      }
    }
    iter->second.push_back(para);
  } else {
    std::vector<PublishPara> temp_vec = {para};
    publish_para_map[keyname] = temp_vec;
  }
  INFO_LOG("insert ins: %s, prid: %s.", keyname.c_str(), para.prid.c_str());

  WriteToJson();
}

void PublishControl::ErasePublishPara(const std::string &keyname, const std::string &ins) {
  for (auto &item_pc : publish_para_map) {
    for (auto iter = item_pc.second.begin(); iter != item_pc.second.end();) {
      if (iter->prid == keyname && (item_pc.first == ins || ins == "")) {
        INFO_LOG("ins: %s, prid: %s doesn't exist anymore, will not subscribe.", item_pc.first.c_str(), keyname.c_str());
        item_pc.second.erase(iter);
      } else {
        iter++;
      }
    }
  }

  WriteToJson();
}

std::vector<utils::InstrumtntID> PublishControl::GetInstrumentList(const std::string &prid) {
  std::vector<utils::InstrumtntID> instrument_vec;
  instrument_vec.clear();

  if (prid == "") {
    for (auto &item_pc : publish_para_map) {
      utils::InstrumtntID item_ins;
      item_ins.ins = item_pc.first;
      item_ins.exch = item_pc.second[0].exch;
      instrument_vec.push_back(item_ins);
    }
  } else {
    for (auto &item_pc : publish_para_map) {
      for (auto &item_id : item_pc.second) {
        if (item_id.prid == prid) {
          utils::InstrumtntID item_ins;
          item_ins.ins = item_pc.first;
          item_ins.exch = item_pc.second[0].exch;
          instrument_vec.push_back(item_ins);
        }
      }
    }
  }
  return instrument_vec;
}

int PublishControl::GetInstrumentSubscribedCount(const std::string &ins) {
  int find_count = 0;

  auto iter = publish_para_map.find(ins);
  if (iter != publish_para_map.end()) {
    find_count = iter->second.size();
  }

  return find_count;
}