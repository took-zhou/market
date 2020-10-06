/*
 * proxyRecer.h
 *
 *  Created on: 2020Äê8ÔÂ29ÈÕ
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_PROXYRECER_H_
#define WORKSPACE_MARKET_INFRA_PROXYRECER_H_

#include "market/infra/define.h"
#include <vector>
struct ZmqBase;

struct ProxyRecer
{
    bool init();
    MsgStruct receMsg();
//    MsgStruct rece();
    bool checkSessionAndTitle(std::vector<std::string>& sessionAndTitle);

    ZmqBase* zmq{nullptr};

};


#endif /* WORKSPACE_MARKET_INFRA_PROXYRECER_H_ */
