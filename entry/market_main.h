#ifndef MARKET_MAIN_H
#define MARKET_MAIN_H
#include <string>

struct MarketMain {
  MarketMain(){};
  ~MarketMain(){};
  MarketMain(const MarketMain &) = delete;
  MarketMain &operator=(const MarketMain &) = delete;
  static MarketMain &GetInstance() {
    static MarketMain instance;
    return instance;
  }

  void Entry(int argc, char *argv[]);
  void HoldOn(void);
  void Exit(void);
  const std::string &GetMarketName();

 private:
  std::string market_name_ = "market";
  bool is_hold_on_ = true;
};

#endif
