#ifndef RTW_HEADER_INSTRUMENTINFO_h_
#define RTW_HEADER_INSTRUMENTINFO_h_

#include <cstdint>
#include <unordered_map>
#include <vector>
#include "common/extern/sqlite3/sqlite3.h"

struct InstrumentInfo {
  struct Info {
    std::string exch;
    double ticksize;
    int32_t max_market_order_volume;
    int32_t min_market_order_volume;
    int32_t max_limit_order_volume;
    int32_t min_limit_order_volume;
    double tradeuint;
    int32_t is_trading;
  };

 public:
  InstrumentInfo();
  ~InstrumentInfo();

  void BuildInstrumentInfo(const std::string &keyname, const Info &info);
  void EraseAllInstrumentInfo(void);
  std::vector<std::string> GetInstrumentList(void);
  std::string GetExchange(const std::string &ins);
  double GetTickSize(const std::string &ins);
  Info *GetInstrumentInfo(const std::string &ins);
  void ShowInstrumentInfo();
  void UpdateInstrumentInfo();

 private:
  void PrepareSqlSentence();
  void RestoreFromSqlite3();
  void InitDatabase();
  std::unordered_map<std::string, Info> info_map_;
  sqlite3_stmt *insert_instrument_ = nullptr;
  sqlite3_stmt *delete_instrument_ = nullptr;
};

#endif