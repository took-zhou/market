#include "market/domain/components/fd_manage.h"
#include "common/extern/log/log.h"
#include "common/extern/sqlite3/sqlite3.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"

FdManage::FdManage() {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  market_data_path = json_cfg.GetConfig("market", "ControlParaFilePath").get<std::string>();
  auto users = json_cfg.GetConfig("market", "User");
  for (auto &user : users) {
    market_data_path = market_data_path + "/" + (std::string)user + "/control.db";
    utils::CreatFile(market_data_path);
    break;
  }

  if (sqlite3_open(market_data_path.c_str(), &market_conn) != SQLITE_OK) {
    ERROR_LOG("Cannot open database: %s\n", sqlite3_errmsg(market_conn));
    sqlite3_close(market_conn);
  }
  sqlite3_exec(market_conn, "PRAGMA synchronous = OFF; ", 0, 0, 0);
  sqlite3_busy_timeout(market_conn, 3000);

  char *error_msg = nullptr;
  if (sqlite3_exec(market_conn, "BEGIN", 0, 0, &error_msg) != SQLITE_OK) {
    ERROR_LOG("Sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(market_conn);
  }
}

void FdManage::OpenThingsUp(void) {
  char *error_msg = nullptr;
  if (sqlite3_exec(market_conn, "COMMIT", 0, 0, &error_msg) != SQLITE_OK) {
    ERROR_LOG("Sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(market_conn);
  }
  if (sqlite3_exec(market_conn, "BEGIN", 0, 0, &error_msg) != SQLITE_OK) {
    ERROR_LOG("Sql error %s.", error_msg);
    sqlite3_free(error_msg);
    sqlite3_close(market_conn);
  }
}
