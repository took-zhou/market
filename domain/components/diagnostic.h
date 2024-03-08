#ifndef MARKET_DIAGNOSTIC_H
#define MARKET_DIAGNOSTIC_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/extern/sqlite3/sqlite3.h"
#include "common/self/dem.h"

enum DiagnosticEventId { kApiCallFailed = 0, kLoginFailed = 1, kStrategyDied = 2, kEventMax = 3 };

struct Diagnostic : Dem {
 public:
  Diagnostic();

  void MonitorStatus();
  void ClearStatus(DiagnosticEventId event_id);
  void RestoreFromSqlite3();

 private:
  void RecordStatus(DiagnosticEventId event_id, DiagEventStatus status);
  void PrepareSqlSentence();
  void InitDatabase();
  std::unordered_map<DiagnosticEventId, bool> diagnostic_recorded_map_;
  const uint32_t sql_length_ = 255;
  sqlite3_stmt* update_diagnostic_info_ = nullptr;
};

#endif
