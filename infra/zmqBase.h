/*
 * zmqBase.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INFRA_ZMQBASE_H_
#define WORKSPACE_MARKET_INFRA_ZMQBASE_H_


struct ZmqBase
{
    ZmqBase(){}
    ZmqBase(const ZmqBase&) = delete;
    ZmqBase& operator=(const ZmqBase&) = delete;
    static ZmqBase& getInstance()
    {
        static ZmqBase instance;
        return instance;
    }
    bool init();
    void SubscribeTopic(const char* topicStr);
    void unSubscribeTopic(const char* topicStr);
    int PublishMsg(const char* head, const char* msg);
    void* context{nullptr};
    void* receiver{nullptr};
    void* publisher{nullptr};
};


#endif /* WORKSPACE_MARKET_INFRA_ZMQBASE_H_ */
