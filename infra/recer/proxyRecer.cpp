/*
 * proxyRecer.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include <map>

#include "market/infra/recer/proxyRecer.h"
#include "market/infra/zmqBase.h"
#include "common/extern/libzmq/include/zhelpers.h"
#include "common/extern/log/log.h"
#include "common/self/utils.h"

extern std::map<std::string, EventType> TitleToEvent;


bool ProxyRecer::init()
{
    topicList.clear();

    // market_strategy
    topicList.push_back("market_strategy.TickSubscribeReq");
    topicList.push_back("market_strategy.TickStartStopIndication");
    topicList.push_back("market_strategy.ActiveSafetyRsp");

    // market_market
    topicList.push_back("market_market.HeartBeat");

    // market_trader
    topicList.push_back("market_trader.QryInstrumentRsq");

    auto& zmqBase = ZmqBase::getInstance();
    for(auto& topic : topicList)
    {
        zmqBase.SubscribeTopic(topic.c_str());
    }

    return true;
}

bool ProxyRecer::checkSessionAndTitle(std::vector<std::string>& sessionAndTitle)
{
    return true;
}

bool ProxyRecer::isTopicInSubTopics(std::string title)
{
    for(auto& topic : topicList)
    {
        if(topic == title)
        {
            return true;
        }
    }
    return false;
}

MsgStruct ProxyRecer::receMsg()
{
    static MsgStruct NilMsgStruct;
    MsgStruct msg;
    auto& zmqBase = ZmqBase::getInstance();
    auto receiver = zmqBase.receiver;
    if(receiver == nullptr)
    {
        ERROR_LOG("receiver is nullptr");
    }
    // INFO_LOG("prepare recv titleChar");
    char* recContent = s_recv(receiver);
    std::string content = std::string(recContent);
    auto spacePos = content.find_first_of(" ");
    auto title = content.substr(0, spacePos);
    auto pbMsg = content.substr(spacePos+1);
    if (!isTopicInSubTopics(title))
    {
        return NilMsgStruct;
    }
    // INFO_LOG("recv msg, topic is[%s]",title.c_str());

    std::string tmpEventName = std::string(title);
    std::vector<std::string> sessionAndTitle = utils::splitString(tmpEventName, std::string("."));
    if (sessionAndTitle.size() != 2)
    {
        return NilMsgStruct;
    }

    if (!checkSessionAndTitle(sessionAndTitle))
    {
        return NilMsgStruct;
    }
    std::string session = sessionAndTitle.at(0);
    std::string msgTitle = sessionAndTitle.at(1);

    msg.sessionName = session;
    msg.msgName = msgTitle;
    msg.pbMsg = pbMsg;
    // INFO_LOG("return msg");
    return msg;
}

