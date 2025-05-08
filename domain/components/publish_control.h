#ifndef MARKET_PUBLISH_CONTROL_H
#define MARKET_PUBLISH_CONTROL_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/extern/sqlite3/sqlite3.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/utils.h"

struct PublishPara {
 public:
  PublishPara(const std::string &exch, uint32_t heartbeat) : exch_(exch), heartbeat_(heartbeat) {}
  const void ClearHeartbeat(void) const { heartbeat_ = 0; };
  uint32_t IncHeartbeat(void) { return ++heartbeat_; };
  const std::string &GetExch() const { return exch_; };

 private:
  std::string exch_ = "";
  mutable uint32_t heartbeat_ = 0;
};

struct PublishControl {
  PublishControl();
  ~PublishControl();

  void BuildPublishPara(const std::string &ins, const PublishPara &para);
  void ErasePublishPara(const std::string &ins = "");
  const PublishPara *GetPublishPara(const std::string &ins);

  const std::vector<utils::InstrumentID> &GetInstrumentList();
  const std::unordered_map<std::string, std::unique_ptr<PublishPara>> &GetPublishParaMap();

 private:
  void PrepareSqlSentence();
  void RestoreFromSqlite3();
  void InitDatabase();
  std::vector<utils::InstrumentID> instrument_vec_;
  std::unordered_map<std::string, std::unique_ptr<PublishPara>> publish_para_map_;
  sqlite3_stmt *insert_control_ = nullptr;
  sqlite3_stmt *delete_control_ = nullptr;
};

#endif
