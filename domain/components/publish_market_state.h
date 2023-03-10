#ifndef PUBLISH_MARKET_STATE_H
#define PUBLISH_MARKET_STATE_H

#include <pthread.h>
#include "common/extern/btp/inc/btp_market_struct.h"

struct PublishState {
 public:
  PublishState();
  // 真实模式
  void PublishEvent(void);
  void PublishToStrategy(void);

  // 回撤模式
  void PublishEvent(BtpLoginLogoutStruct *login_logout);
  void PublishToStrategy(BtpLoginLogoutStruct *login_logout);

  void ClearPublishFlag();
  void SetPublishFlag();
  ~PublishState(){};

 private:
  void GetTradeData(char *);
  int IsLeapYear(int year);
  bool publish_flag_ = false;
  uint32_t wait_publish_count_ = 0;
  const uint32_t max_wait_pushlish_count_ = 3600;
  pthread_mutex_t sm_mutex_;
};

#endif