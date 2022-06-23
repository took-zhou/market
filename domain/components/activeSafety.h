#ifndef ACTIVE_SAFETY_H
#define ACTIVE_SAFETY_H

#include <string>

struct activeSafety {
  activeSafety();

  // 开启查询线程
  // 制定查询超时策略
  ~activeSafety(){};

  void checkSafety();

  void req_alive();
  void req_alive_timeout(const std::string& keyname);
};

#endif
