/*
 * proxySender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/proxySender.h"

#include "market/infra/zmqBase.h"

#include "common/extern/log/log.h"

bool ProxySender::init()
{
    auto& zmqBase = ZmqBase::getInstance();
    zmq = &zmqBase;

    return true;
}

bool ProxySender::send(const char* head, const char* msg)
{
    auto& zmqBase = ZmqBase::getInstance();
    return zmqBase.PublishMsg(head, msg);
}
