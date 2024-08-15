#include <stdio.h>
#include <sys/statvfs.h>
#include <time.h>
#include <unistd.h>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

#include "common/extern/log/log.h"
#include "common/self/dem.h"
#include "market/domain/components/diagnostic.h"
#include "market/domain/components/fd_manage.h"

Diagnostic::Diagnostic() {
  AddTestConfig(kApiCallFailed, 10, 10, DiagEventStatus::kPass);
  AddTestConfig(kLoginFailed, 10, 10, DiagEventStatus::kPass);

  InitDatabase();
  PrepareSqlSentence();
}

Diagnostic::~Diagnostic() { sqlite3_finalize(update_diagnostic_info_); }

void Diagnostic::InitDatabase() {
  const char* sql = "create table if not exists diagnostic_info(id INT, status INT, time TEXT)";
  char* error_msg = nullptr;
  if (sqlite3_exec(FdManage::GetInstance().GetMarketConn(), sql, NULL, NULL, &error_msg) != SQLITE_OK) {
    ERROR_LOG("Sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }

  char sql2[sql_length_];
  for (auto& item : GetTestConfig()) {
    snprintf(sql2, sql_length_,
             "insert into diagnostic_info(id, status, time) select %d, 0, '' where not exists (select * from diagnostic_info where id=%d);",
             item.event_id, item.event_id);
    if (sqlite3_exec(FdManage::GetInstance().GetMarketConn(), sql2, NULL, NULL, &error_msg) != SQLITE_OK) {
      sqlite3_free(error_msg);
      sqlite3_close(FdManage::GetInstance().GetMarketConn());
    }
  }
}

void Diagnostic::PrepareSqlSentence() {
  const char* sql = "update diagnostic_info set status = ?, time = ? where id = ?;";
  if (sqlite3_prepare_v2(FdManage::GetInstance().GetMarketConn(), sql, strlen(sql), &update_diagnostic_info_, 0) != SQLITE_OK) {
    ERROR_LOG("prepare sql sentence error.");
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }
}

void Diagnostic::MonitorStatus() {
  uint64_t event_id_mask = ReadEventMask();
  for (uint16_t index = 0; index < kEventMax; index++) {
    if (event_id_mask == 0) {
      break;
    }
    if ((event_id_mask & 1) && !diagnostic_recorded_map_[static_cast<DiagnosticEventId>(index)]) {
      RecordStatus(static_cast<DiagnosticEventId>(index), DiagEventStatus::kFail);
      diagnostic_recorded_map_[static_cast<DiagnosticEventId>(index)] = true;
    }
    event_id_mask >>= 1;
  }
}

void Diagnostic::ClearStatus(DiagnosticEventId event_id) {
  if (event_id >= kEventMax) {
    return;
  }
  diagnostic_recorded_map_[event_id] = false;
  RecordStatus(event_id, DiagEventStatus::kTestNocompleted);
  INFO_LOG("clear diagnostic event id %d' status ok.", event_id);
}

void Diagnostic::RecordStatus(DiagnosticEventId event_id, DiagEventStatus status) {
  time_t now = time(0);
  tm* local_time = localtime(&now);
  char snapshot_time[24];
  strftime(snapshot_time, sizeof(snapshot_time), "%Y-%m-%d %H:%M:%S", local_time);

  sqlite3_reset(update_diagnostic_info_);
  sqlite3_bind_int(update_diagnostic_info_, 1, status);
  sqlite3_bind_text(update_diagnostic_info_, 2, snapshot_time, sizeof(snapshot_time), 0);
  sqlite3_bind_int(update_diagnostic_info_, 3, event_id);
  if (sqlite3_step(update_diagnostic_info_) != SQLITE_DONE) {
    ERROR_LOG("do sql sentence error.");
    sqlite3_close(FdManage::GetInstance().GetMarketConn());
  }
}
