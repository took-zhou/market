#ifndef MARKET_PUBLISH_CONTROL_H
#define MARKET_PUBLISH_CONTROL_H

#include <string>
#include <unordered_map>
#include <vector>

#include "common/extern/sqlite3/sqlite3.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"

struct PublishPara {
  /*realtime*/
  std::string exch = "";
  mutable uint32_t heartbeat = 0;
};

struct PublishControl {
  PublishControl();
  ~PublishControl();

  void BuildPublishPara(const std::string &ins, const PublishPara &para);
  void ErasePublishPara(const std::string &ins = "");

  std::vector<utils::InstrumtntID> GetInstrumentList();
  std::unordered_map<std::string, PublishPara> publish_para_map;
  bool init_database_flag = false;

 private:
  void PrepareSqlSentence();
  void RestoreFromSqlite3();
  void InitDatabase();
  sqlite3_stmt *insert_control_ = nullptr;
  sqlite3_stmt *delete_control_ = nullptr;
};

#endif
