/*
 * zmqBase.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/zmqBase.h"
#include "common/self/fileUtil.h"
#include "common/extern/libzmq/include/zhelpers.h"
#include "common/extern/log/log.h"
#include "common/self/basetype.h"

#include <string>
#include <sstream>
#include <unistd.h>

using json = nlohmann::json;

constexpr U32 WAITTIME_FOR_ZMQ_INIT = 1;

bool ZmqBase::init()
{
    context = zmq_ctx_new();
    receiver = zmq_socket(context, ZMQ_SUB);
    publisher = zmq_socket(context, ZMQ_PUB);
    auto& jsonCfg = utils::JsonConfig::getInstance();
    std::string netStr = jsonCfg.getConfig("market", "SubAddPort").get<std::string>();
    int result = zmq_connect(receiver, netStr.c_str());
    sleep(WAITTIME_FOR_ZMQ_INIT);
    INFO_LOG("result = %d",result);
    if(result != 0)
    {
        ERROR_LOG("receiver connect to %s failed",netStr.c_str());
        return false;
    }
    netStr = jsonCfg.getConfig("market", "PubAddPort").get<std::string>();

    result = zmq_connect(publisher, netStr.c_str());
    INFO_LOG("result = %d",result);
    if(result != 0)
    {
        ERROR_LOG("publisher connect to %s failed",netStr.c_str());
        return false;
    }
    INFO_LOG("zmq init ok");
    return true;
}

void ZmqBase::SubscribeTopic(const char* topicStr)
{
    zmq_setsockopt(receiver, ZMQ_SUBSCRIBE, topicStr, strlen(topicStr));
}

void ZmqBase::unSubscribeTopic(const char* topicStr)
{
    zmq_setsockopt(receiver, ZMQ_UNSUBSCRIBE, topicStr, strlen(topicStr));
}

int ZmqBase::PublishMsg(const char* head, const char* msg)
{
    std::stringstream  tmpStr;
    tmpStr << head << " " << msg;
    int ret2 = s_send(publisher, const_cast<char*>(tmpStr.str().c_str()));
    return ret2;
}


