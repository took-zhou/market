#include "market/domain/components/fd_manage.h"
#include "common/extern/log/log.h"
#include "common/extern/sqlite3/sqlite3.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"

FdManage::FdManage() {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  market_data_path_ = json_cfg.GetConfig("market", "ControlParaFilePath").get<std::string>();
  auto users = json_cfg.GetConfig("market", "User");
  for (auto &user : users) {
    market_data_path_ = market_data_path_ + "/" + static_cast<std::string>(user) + "/control.db";
    utils::CreatFile(market_data_path_);
    break;
  }

  if (sqlite3_open(market_data_path_.c_str(), &market_conn_) != SQLITE_OK) {
    ERROR_LOG("cannot open database: %s\n", sqlite3_errmsg(market_conn_));
    sqlite3_close(market_conn_);
  }
  sqlite3_exec(market_conn_, "PRAGMA synchronous = OFF; ", 0, 0, 0);
  sqlite3_busy_timeout(market_conn_, 3000);

  char *error_msg = nullptr;
  if (sqlite3_exec(market_conn_, "BEGIN", 0, 0, &error_msg) != SQLITE_OK) {
    ERROR_LOG("sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(market_conn_);
  }
}

FdManage::~FdManage() {
  char *error_msg = nullptr;
  if (sqlite3_exec(market_conn_, "COMMIT", 0, 0, &error_msg) != SQLITE_OK) {
    ERROR_LOG("sql error %s.", error_msg);
    sqlite3_free(error_msg);
  }
  sqlite3_close(market_conn_);
  INFO_LOG("exec commit, close market conn ok.");
}

void FdManage::OpenThingsUp(void) {
  char *error_msg = nullptr;
  if (sqlite3_exec(market_conn_, "COMMIT", 0, 0, &error_msg) != SQLITE_OK) {
    ERROR_LOG("sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(market_conn_);
  }
  if (sqlite3_exec(market_conn_, "BEGIN", 0, 0, &error_msg) != SQLITE_OK) {
    ERROR_LOG("sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(market_conn_);
  }
}

sqlite3 *FdManage::GetMarketConn(void) { return market_conn_; }