#ifndef PUBLISH_MARKET_STATE_H
#define PUBLISH_MARKET_STATE_H

struct PublishState {
 public:
  PublishState();
  void PublishEvent(void);

  void PublishToStrategy(void);
  void PublishToManage(void);
  void GetTradeData(char *);

  ~PublishState(){};

 private:
  int IsLeapYear(int year);
};

#endif