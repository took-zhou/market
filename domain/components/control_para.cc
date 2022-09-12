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
using fifo_workaround_fifo_map = fifo_map<K, V, fifo_map_compare<K>, A>;
using fifo_json = basic_json<fifo_workaround_fifo_map>;

ControlPara::ControlPara(void) {
  auto &jsonCfg = utils::JsonConfig::getInstance();
  auto users = jsonCfg.get_config("market", "User");
  for (auto &user : users) {
    std::string temp_folder = jsonCfg.get_config("market", "ControlParaFilePath").get<std::string>();
    json_path = temp_folder + "/" + (std::string)user + "/controlPara/control.json";
    if (!utils::IsFileExist(json_path)) {
      utils::CreatFile(json_path);
    } else if (utils::get_file_size(json_path) > 0) {
      LoadFromJson();
    }
    break;
  }
}

bool ControlPara::LoadFromJson(void) {
  int ret = true;
  fifo_json readData;
  ifstream outFile(json_path, ios::binary);
  if (outFile.is_open()) {
    outFile >> readData;
    for (auto iter = readData.begin(); iter != readData.end(); iter++) {
      std::vector<PublishControl> tempcontrol_vec;
      for (int i = 0; i < readData[iter.key()].size(); i++) {
        PublishControl tempControl;
        readData[iter.key()][i].at("exch").get_to(tempControl.exch);
        readData[iter.key()][i].at("ticksize").get_to(tempControl.ticksize);
        readData[iter.key()][i].at("prid").get_to(tempControl.prid);
        readData[iter.key()][i].at("indication").get_to(tempControl.indication);
        readData[iter.key()][i].at("interval").get_to(tempControl.interval);
        readData[iter.key()][i].at("directforward").get_to(tempControl.directforward);
        readData[iter.key()][i].at("source").get_to(tempControl.source);

        tempcontrol_vec.push_back(tempControl);
        INFO_LOG("load prid: %s, instrument: %s.", tempControl.prid.c_str(), iter.key().c_str());
      }
      publish_ctrl_map.insert(make_pair(iter.key(), tempcontrol_vec));
    }
  } else {
    WARNING_LOG("file:%s not exist.", json_path.c_str());
    ret = false;
  }
  outFile.close();

  return ret;
}

bool ControlPara::WriteToJson(void) {
  int ret = true;
  fifo_json writeData;

  for (auto &item_pc : publish_ctrl_map) {
    for (auto &item_id : item_pc.second) {
      fifo_json one_item;
      one_item["exch"] = item_id.exch;
      one_item["prid"] = item_id.prid;
      one_item["ticksize"] = item_id.ticksize;
      one_item["indication"] = item_id.indication;
      one_item["interval"] = item_id.interval;
      one_item["source"] = item_id.source;
      one_item["directforward"] = item_id.directforward;
      writeData[item_pc.first].push_back(one_item);
    }
  }

  ofstream inFile(json_path);
  if (inFile.is_open()) {
    inFile << setw(4) << writeData << endl;
  } else {
    ERROR_LOG("file:%s open error.", json_path.c_str());
    ret = false;
  }
  inFile.close();

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

void ControlPara::EraseControlPara(const std::string &keyname) {
  for (auto &item_pc : publish_ctrl_map) {
    for (auto iter = item_pc.second.begin(); iter != item_pc.second.end();) {
      if (iter->prid == keyname) {
        INFO_LOG("ins: %s, prid: %s req alive timeout, will not subscribe.", item_pc.first.c_str(), keyname.c_str());
        item_pc.second.erase(iter);
      } else {
        iter++;
      }
    }
  }

  WriteToJson();
}

void ControlPara::set_start_stop_indication(const std::string keyname, strategy_market::TickStartStopIndication_MessageType _indication) {
  for (auto &item_pc : publish_ctrl_map) {
    for (auto &item_id : item_pc.second) {
      if (item_id.prid == keyname) {
        item_id.indication = _indication;
        INFO_LOG("ins: %s, prid: %s, setStartStopIndication %d.", item_pc.first.c_str(), keyname.c_str(), _indication);
      }
    }
  }

  WriteToJson();
}

std::vector<utils::InstrumtntID> ControlPara::get_instrument_list(void) {
  std::vector<utils::InstrumtntID> instrument_vec;
  instrument_vec.clear();

  for (auto &item_pc : publish_ctrl_map) {
    utils::InstrumtntID item_ins;
    item_ins.ins = item_pc.first;
    item_ins.exch = item_pc.second[0].exch;
    item_ins.ticksize = item_pc.second[0].ticksize;
    instrument_vec.push_back(item_ins);
  }

  return instrument_vec;
}

std::vector<std::string> ControlPara::get_prid_list(void) {
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
