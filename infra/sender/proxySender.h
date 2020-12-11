/*
 * proxySender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_TRADER_INFRA_PROXYSENDER_H_
#define WORKSPACE_TRADER_INFRA_PROXYSENDER_H_

struct ZmqBase;
struct ProxySender
{
    bool init();
    bool send(const char* head, const char* msg);
    ZmqBase* zmq{nullptr};
};


#endif /* WORKSPACE_TRADER_INFRA_PROXYSENDER_H_ */
