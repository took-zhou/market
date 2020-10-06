/*
 * proxySender.cpp
 *
 *  Created on: 2020Äê8ÔÂ30ÈÕ
 *      Author: Administrator
 */

#include "market/infra/sender/proxySender.h"

#include "market/infra/zmqBase.h"

bool ProxySender::init()
{
    auto& zmqBase = ZmqBase::getInstance();
    zmq = &zmqBase;

    return true;
}

bool ProxySender::send()
{
    return true;
}
