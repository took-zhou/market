#ifndef PUBLISH_MARKET_STATE_H
#define PUBLISH_MARKET_STATE_H

struct publishState {
 public:
  publishState();
  void publish_event(void);

  void publish_to_strategy(void);
  void publish_to_manage(void);
  void get_trade_data(char *);

  ~publishState(){};

 private:
  int is_leap_year(int y);
};

#endif