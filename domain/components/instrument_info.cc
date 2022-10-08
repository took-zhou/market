#include "market/domain/components/instrument_info.h"
#include "common/extern/log/log.h"

void InstrumentInfo::BuildInstrumentInfo(const std::string &keyname, const Info &info) {
  auto iter = info_map_.find(keyname);
  if (iter != info_map_.end()) {
    INFO_LOG("update instrument info: %s.", keyname.c_str());
  } else {
    INFO_LOG("build instrument info: %s.", keyname.c_str());
  }
  info_map_[keyname] = info;
}

void InstrumentInfo::EraseAllInstrumentInfo(void) {
  info_map_.clear();
  INFO_LOG("erase all instrument info.");
}

std::vector<std::string> InstrumentInfo::GetInstrumentList(void) {
  std::vector<std::string> ret_vec;
  for (auto &item : info_map_) {
    ret_vec.push_back(item.first);
  }

  return ret_vec;
}

std::string InstrumentInfo::GetExchange(const std::string &ins) {
  auto iter = info_map_.find(ins);
  if (iter != info_map_.end()) {
    return iter->second.exch;
  } else {
    ERROR_LOG("not find ins: %s", ins.c_str());
    return "";
  }
}

float InstrumentInfo::GetTickSize(const std::string &ins) {
  auto iter = info_map_.find(ins);
  if (iter != info_map_.end()) {
    return iter->second.ticksize;
  } else {
    ERROR_LOG("not find ins: %s", ins.c_str());
    return -1;
  }
}

InstrumentInfo::Info *InstrumentInfo::GetInstrumentInfo(const std::string &ins) {
  auto iter = info_map_.find(ins);
  if (iter != info_map_.end()) {
    return &iter->second;
  } else {
    ERROR_LOG("not find ins: %s", ins.c_str());
    return nullptr;
  }
}