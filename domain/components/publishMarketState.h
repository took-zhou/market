#ifndef PUBLISH_MARKET_STATE_H
#define PUBLISH_MARKET_STATE_H

struct publishState{
public:
    publishState();
    void publish_event(void);
    void pushlish_cycle(void);
    void get_trade_data(char *);
    ~publishState() {};
private:
    void publish_day_closing(void);
    int is_leap_year(int y);
};

#endif