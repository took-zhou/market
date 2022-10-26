#ifndef MARKET_ACTIVE_SAFETY_H
#define MARKET_ACTIVE_SAFETY_H

#include <string>

struct ActiveSafety {
  ActiveSafety();

  // 开启查询线程
  // 制定查询超时策略
  ~ActiveSafety(){};

  void CheckSafety();

  void ReqAlive();
  void ReqAliveTimeout(const std::string& keyname);
};

#endif
