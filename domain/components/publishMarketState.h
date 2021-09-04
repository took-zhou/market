#ifndef PUBLISH_MARKET_STATE_H
#define PUBLISH_MARKET_STATE_H

struct publishState{
public:
    publishState();
    void publish(void);
    ~publishState() {};
private:
    unsigned int publish_count = 0;
};

#endif