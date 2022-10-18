#ifndef PUBLISH_MARKET_STATE_H
#define PUBLISH_MARKET_STATE_H

#include "common/extern/btp/inc/btp_market_struct.h"

struct PublishState {
 public:
  PublishState();
  void PublishEvent(void);
  void PublishToStrategy(void);

  void PublishEvent(BtpLoginLogoutStruct *login_logout);
  void PublishToStrategy(BtpLoginLogoutStruct *login_logout);

  ~PublishState(){};

 private:
  void GetTradeData(char *);
  int IsLeapYear(int year);
};

#endif