#ifndef MARKET_PYTHON_API_H
#define MARKET_PYTHON_API_H

#include <string>

struct TradeDate {
  bool GetLoginDate(const std::string &timestring, char *login_date);
};

struct TickNature {
 public:
  TradeDate &GetTradeDate() { return trade_date_; };

 private:
  TradeDate trade_date_;
};

struct PythonApi {
 public:
  TickNature &GetTickNature() { return tick_nature_; };
  bool ApplyThreadPatch();

 private:
  TickNature tick_nature_;
};

#endif