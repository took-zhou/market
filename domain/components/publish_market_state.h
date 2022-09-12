#ifndef PUBLISH_MARKET_STATE_H
#define PUBLISH_MARKET_STATE_H

struct PublishState {
 public:
  PublishState();
  void publish_event(void);

  void publish_to_strategy(void);
  void publish_to_manage(void);
  void get_trade_data(char *);

  ~PublishState(){};

 private:
  int is_leap_year(int y);
};

#endif