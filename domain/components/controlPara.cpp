#include <fstream>
#include <iomanip>

#include "market/domain/components/controlPara.h"

#include "common/extern/json/fifo_map.hpp"
#include "common/extern/json/json.h"
#include "common/extern/log/log.h"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"

using namespace nlohmann;
template <class K, class V, class dummy_compare, class A>
using fifo_workaround_fifo_map = fifo_map<K, V, fifo_map_compare<K>, A>;
using fifo_json = basic_json<fifo_workaround_fifo_map>;

controlPara::controlPara(void) {
  auto &jsonCfg = utils::JsonConfig::getInstance();
  json_path = jsonCfg.getConfig("market", "ControlParaFilePath").get<std::string>();
  if (!utils::isFileExist(json_path)) {
    utils::creatFile(json_path);
  } else if (utils::getFileSize(json_path) > 0) {
    load_from_json();
  }
}

bool controlPara::load_from_json(void) {
  int ret = true;
  fifo_json readData;
  ifstream outFile(json_path, ios::binary);
  if (outFile.is_open()) {
    outFile >> readData;
    for (auto iter = readData.begin(); iter != readData.end(); iter++) {
      std::vector<publishControl> tempcontrol_vec;
      for (int i = 0; i < readData[iter.key()].size(); i++) {
        publishControl tempControl;
        readData[iter.key()][i].at("exch").get_to(tempControl.exch);
        readData[iter.key()][i].at("ticksize").get_to(tempControl.ticksize);
        readData[iter.key()][i].at("identify").get_to(tempControl.identify);
        readData[iter.key()][i].at("indication").get_to(tempControl.indication);
        readData[iter.key()][i].at("interval").get_to(tempControl.interval);
        readData[iter.key()][i].at("directforward").get_to(tempControl.directforward);
        readData[iter.key()][i].at("source").get_to(tempControl.source);

        tempcontrol_vec.push_back(tempControl);
        INFO_LOG("load identify: %s, instrument: %s.", tempControl.identify.c_str(), iter.key().c_str());
      }
      publishCtrlMap.insert(make_pair(iter.key(), tempcontrol_vec));
    }
  } else {
    WARNING_LOG("file:%s not exist.", json_path.c_str());
    ret = false;
  }
  outFile.close();

  return ret;
}

bool controlPara::write_to_json(void) {
  int ret = true;
  fifo_json writeData;

  for (auto &item_pc : publishCtrlMap) {
    for (auto &item_id : item_pc.second) {
      fifo_json one_item;
      one_item["exch"] = item_id.exch;
      one_item["identify"] = item_id.identify;
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

void controlPara::buildControlPara(const std::string &keyname, const publishControl &para) {
  auto iter = publishCtrlMap.find(keyname);
  if (iter != publishCtrlMap.end()) {
    for (auto &item : iter->second) {
      if (item.identify == para.identify) {
        return;
      }
    }
    iter->second.push_back(para);
  } else {
    std::vector<publishControl> temp_vec = {para};
    publishCtrlMap[keyname] = temp_vec;
  }
  INFO_LOG("insert ins: %s, identify: %s.", keyname.c_str(), para.identify.c_str());

  write_to_json();
}

void controlPara::eraseControlPara(const std::string &keyname) {
  for (auto &item_pc : publishCtrlMap) {
    for (auto iter = item_pc.second.begin(); iter != item_pc.second.end();) {
      if (iter->identify == keyname) {
        INFO_LOG("ins: %s, identify: %s req alive timeout, will not subscribe.", item_pc.first.c_str(), keyname.c_str());
        item_pc.second.erase(iter);
      } else {
        iter++;
      }
    }
  }

  write_to_json();
}

void controlPara::setStartStopIndication(const std::string keyname, strategy_market::TickStartStopIndication_MessageType _indication) {
  for (auto &item_pc : publishCtrlMap) {
    for (auto &item_id : item_pc.second) {
      if (item_id.identify == keyname) {
        item_id.indication = _indication;
        INFO_LOG("ins: %s, identify: %s, setStartStopIndication %d.", item_pc.first.c_str(), keyname.c_str(), _indication);
      }
    }
  }

  write_to_json();
}

std::vector<utils::InstrumtntID> controlPara::getInstrumentList(void) {
  std::vector<utils::InstrumtntID> instrument_vec;
  instrument_vec.clear();

  for (auto &item_pc : publishCtrlMap) {
    utils::InstrumtntID item_ins;
    item_ins.ins = item_pc.first;
    item_ins.exch = item_pc.second[0].exch;
    item_ins.ticksize = item_pc.second[0].ticksize;
    instrument_vec.push_back(item_ins);
  }

  return instrument_vec;
}

std::vector<std::string> controlPara::getIdentifyList(void) {
  std::vector<std::string> temp_vec;
  temp_vec.clear();

  std::set<std::string> temp_set;
  temp_set.clear();

  for (auto &item_pc : publishCtrlMap) {
    for (auto &item_id : item_pc.second) {
      temp_set.insert(item_id.identify);
    }
  }

  temp_vec.assign(temp_set.begin(), temp_set.end());
  return temp_vec;
}
