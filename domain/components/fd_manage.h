#ifndef MARKET_FD_MANAGE_H
#define MARKET_FD_MANAGE_H
#include <memory>
#include <string>
#include "common/extern/sqlite3/sqlite3.h"

struct FdManage {
 public:
  FdManage();
  ~FdManage();
  FdManage(const FdManage &) = delete;
  FdManage &operator=(const FdManage &) = delete;
  static FdManage &GetInstance() {
    static FdManage instance;
    return instance;
  }

  void OpenThingsUp(void);
  sqlite3 *GetMarketConn(void);

 private:
  sqlite3 *market_conn_ = nullptr;
  std::string market_data_path_ = "";
};

#endif