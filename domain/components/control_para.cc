#include <fstream>
#include <iomanip>

#include "market/domain/components/control_para.h"

#include "common/extern/json/fifo_map.hpp"
#include "common/extern/json/json.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"

using namespace nlohmann;
template <class K, class V, class dummy_compare, class A>
using FifoWorkaroundFifoMap = fifo_map<K, V, fifo_map_compare<K>, A>;
using FifoJson = basic_json<FifoWorkaroundFifoMap>;

ControlPara::ControlPara(void) {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto users = json_cfg.GetConfig("market", "User");
  for (auto &user : users) {
    std::string temp_folder = json_cfg.GetConfig("market", "ControlParaFilePath").get<std::string>();
    json_path_ = temp_folder + "/" + (std::string)user + "/controlPara/control.json";
    if (!utils::IsFileExist(json_path_)) {
      utils::CreatFile(json_path_);
    } else if (utils::GetFileSize(json_path_) > 0) {
      LoadFromJson();
    }
    break;
  }
}

bool ControlPara::LoadFromJson(void) {
  int ret = true;
  FifoJson read_data;
  ifstream out_file(json_path_, ios::binary);
  if (out_file.is_open()) {
    out_file >> read_data;
    for (auto iter = read_data.begin(); iter != read_data.end(); iter++) {
      std::vector<PublishControl> tempcontrol_vec;
      for (int i = 0; i < read_data[iter.key()].size(); i++) {
        PublishControl temp_control;
        read_data[iter.key()][i].at("exch").get_to(temp_control.exch);
        read_data[iter.key()][i].at("prid").get_to(temp_control.prid);
        read_data[iter.key()][i].at("indication").get_to(temp_control.indication);
        read_data[iter.key()][i].at("interval").get_to(temp_control.interval);
        read_data[iter.key()][i].at("directforward").get_to(temp_control.directforward);
        read_data[iter.key()][i].at("source").get_to(temp_control.source);
        read_data[iter.key()][i].at("begin").get_to(temp_control.begin);
        read_data[iter.key()][i].at("end").get_to(temp_control.end);
        read_data[iter.key()][i].at("speed").get_to(temp_control.speed);

        tempcontrol_vec.push_back(temp_control);
        INFO_LOG("load prid: %s, instrument: %s.", temp_control.prid.c_str(), iter.key().c_str());
      }
      publish_ctrl_map.insert(make_pair(iter.key(), tempcontrol_vec));
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

  for (auto &item_pc : publish_ctrl_map) {
    for (auto &item_id : item_pc.second) {
      FifoJson one_item;
      one_item["exch"] = item_id.exch;
      one_item["prid"] = item_id.prid;
      one_item["indication"] = item_id.indication;
      one_item["interval"] = item_id.interval;
      one_item["source"] = item_id.source;
      one_item["directforward"] = item_id.directforward;
      one_item["begin"] = item_id.begin;
      one_item["end"] = item_id.end;
      one_item["speed"] = item_id.speed;
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

void ControlPara::BuildControlPara(const std::string &keyname, const PublishControl &para) {
  auto iter = publish_ctrl_map.find(keyname);
  if (iter != publish_ctrl_map.end()) {
    for (auto &item : iter->second) {
      if (item.prid == para.prid) {
        return;
      }
    }
    iter->second.push_back(para);
  } else {
    std::vector<PublishControl> temp_vec = {para};
    publish_ctrl_map[keyname] = temp_vec;
  }
  INFO_LOG("insert ins: %s, prid: %s.", keyname.c_str(), para.prid.c_str());

  WriteToJson();
}

void ControlPara::EraseControlPara(const std::string &keyname, const std::string &ins) {
  for (auto &item_pc : publish_ctrl_map) {
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

void ControlPara::SetStartStopIndication(const std::string &keyname, const std::string &ins,
                                         strategy_market::TickStartStopIndication_MessageType indication) {
  auto iter = publish_ctrl_map.find(ins);
  if (iter != publish_ctrl_map.end()) {
    for (auto &item_id : iter->second) {
      if (item_id.prid == keyname) {
        item_id.indication = indication;
        INFO_LOG("ins: %s, prid: %s, setStartStopIndication %d.", ins.c_str(), keyname.c_str(), indication);
      }
    }

    WriteToJson();
  } else {
    INFO_LOG("not find ins: %s.", ins.c_str());
  }
}

std::vector<utils::InstrumtntID> ControlPara::GetInstrumentList(const std::string &prid) {
  std::vector<utils::InstrumtntID> instrument_vec;
  instrument_vec.clear();

  if (prid == "") {
    for (auto &item_pc : publish_ctrl_map) {
      utils::InstrumtntID item_ins;
      item_ins.ins = item_pc.first;
      item_ins.exch = item_pc.second[0].exch;
      instrument_vec.push_back(item_ins);
    }
  } else {
    for (auto &item_pc : publish_ctrl_map) {
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

std::vector<std::string> ControlPara::GetPridList(void) {
  std::vector<std::string> temp_vec;
  temp_vec.clear();

  std::set<std::string> temp_set;
  temp_set.clear();

  for (auto &item_pc : publish_ctrl_map) {
    for (auto &item_id : item_pc.second) {
      temp_set.insert(item_id.prid);
    }
  }

  temp_vec.assign(temp_set.begin(), temp_set.end());
  return temp_vec;
}

int ControlPara::GetInstrumentSubscribedCount(const utils::InstrumtntID &instruemtn_id) {
  int find_count = 0;

  for (auto &item_pc : publish_ctrl_map) {
    if (item_pc.first == instruemtn_id.ins) {
      find_count = find_count + 1;
    }
  }

  return find_count;
}