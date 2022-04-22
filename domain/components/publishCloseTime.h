#ifndef PUBLISH_CLOSE_TIME_H
#define PUBLISH_CLOSE_TIME_H

struct publishCloseTime
{
    publishCloseTime();
    ~publishCloseTime() {};

    void publish(void);

    bool getLocalHMS(char *t_arr);
};

#endif