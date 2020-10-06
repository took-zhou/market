/*
 * interactRecer.h
 *
 *  Created on: 2020Äê8ÔÂ29ÈÕ
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_INTERACTRECER_H_
#define WORKSPACE_MARKET_INFRA_INTERACTRECER_H_
#include "market/infra/define.h"
#include "market/infra/zmqBase.h"
struct InteractRecer
{
    bool init(){
        return true;
    };

    MsgStruct rece();

    ZmqBase* zmq{nullptr};
};



#endif /* WORKSPACE_MARKET_INFRA_INTERACTRECER_H_ */
