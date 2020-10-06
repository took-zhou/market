/*
 * proxySender.h
 *
 *  Created on: 2020Äê8ÔÂ29ÈÕ
 *      Author: Administrator
 */

#ifndef WORKSPACE_TRADER_INFRA_PROXYSENDER_H_
#define WORKSPACE_TRADER_INFRA_PROXYSENDER_H_

struct ZmqBase;
struct ProxySender
{
    bool init();
    bool send();
    ZmqBase* zmq{nullptr};
};


#endif /* WORKSPACE_TRADER_INFRA_PROXYSENDER_H_ */
