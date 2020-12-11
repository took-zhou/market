/*
 * marketLoginState.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/domain/components/ctpMarketApi/marketLoginState.h"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"
#include "common/extern/log/log.h"
#include <string>
#include <string.h>
#include <unistd.h>

#define IN_day_login                   ((U8)1U)
#define IN_day_logout                  ((U8)2U)
#define IN_init_sts                    ((U8)3U)
#define IN_night_login                 ((U8)4U)
#define IN_night_logout                ((U8)5U)

U32 MarketLoginState::isDuringDayLogoutTime(void)
{
    U32 out;
    out = 0U;

    if ((strcmp(input.loginTime, "night") != 0) && (strcmp(input.loginMode, "normal") ==
        0) && ((input.day_logout_mins < input.now_mins) && (input.now_mins <
          input.night_login_mins)))
    {
        out = 1U;
    }

    return out;
}

U32 MarketLoginState::isDuringNightLogoutTime(void)
{
    U32 out;
    out = 0U;

    if ((strcmp(input.loginTime, "day") != 0) && (strcmp(input.loginMode, "normal") ==
        0) && ((input.night_logout_mins < input.now_mins) && (input.now_mins <
          input.day_login_mins)))
    {
        out = 1U;
    }

    return out;
}

U32 MarketLoginState::isDuringDayLoginTime(void)
{
    U32 out;
    out = 0U;

    if ((strcmp(input.loginTime, "night") != 0) && (strcmp(input.loginMode, "normal") ==
        0) && ((input.day_login_mins <= input.now_mins) && (input.now_mins <=
          input.day_logout_mins)))
    {
        out = 1U;
    }

    return out;
}

U32 MarketLoginState::isDuringNightLoginTime(void)
{
    U32 out;
    out = 0U;
    if ((strcmp(input.loginTime, "day") != 0) && (strcmp(input.loginMode, "normal") ==
          0) && ((input.night_login_mins <= input.now_mins) || (input.now_mins <=
          input.night_logout_mins)))
    {
        out = 1U;
    }
    return out;
}

void MarketLoginState::determineLoginMode(void)
{
    if (strcmp(input.loginMode, "normal") == 0)
    {
        output.status = LOGOUT_TIME;
    }
    else
    {
        output.status = LOGIN_TIME;
    }
}

// Model step function
void MarketLoginState::step()
{
    if (rtDW.is_active_MarketLoginState == 0U) 
    {
        rtDW.is_active_MarketLoginState = 1U;
        rtDW.is_MarketLoginState = IN_init_sts;
        determineLoginMode();
    }
    else
    {
        switch (rtDW.is_MarketLoginState)
        {
            case IN_day_login:
                if (input.now_mins == input.day_logout_mins)
                {
                    rtDW.is_MarketLoginState = IN_day_logout;
                    output.status = LOGOUT_TIME;
                }
                break;

            case IN_day_logout:
                if ((strcmp(input.loginTime, "day") != 0) && (input.now_mins == input.night_login_mins))
                {
                    rtDW.is_MarketLoginState = IN_night_login;
                    output.status = LOGIN_TIME;
                }
                else
                {
                    if ((strcmp(input.loginTime, "day") == 0) && (input.now_mins == input.day_login_mins))
                    {
                        rtDW.is_MarketLoginState = IN_day_login;
                        output.status = LOGIN_TIME;
                    }
                }
                break;

            case IN_init_sts:
                if (isDuringDayLogoutTime() != 0U)
                {
                    rtDW.is_MarketLoginState = IN_day_logout;
                    output.status = LOGOUT_TIME;
                } 
                else if (isDuringNightLogoutTime() != 0U)
                {
                    rtDW.is_MarketLoginState = IN_night_logout;
                    output.status = LOGOUT_TIME;
                }
                else if (isDuringDayLoginTime() != 0U)
                {
                    rtDW.is_MarketLoginState = IN_day_login;
                    output.status = LOGIN_TIME;
                }
                else
                {
                    if (isDuringNightLoginTime() != 0U)
                    {
                        rtDW.is_MarketLoginState = IN_night_login;
                        output.status = LOGIN_TIME;
                    }
                }
                break;

            case IN_night_login:
                if (input.now_mins == input.night_logout_mins)
                {
                    rtDW.is_MarketLoginState = IN_night_logout;
                    output.status = LOGOUT_TIME;
                }
                break;

            default:
                if ((strcmp(input.loginTime, "night") != 0) && (input.now_mins == input.day_login_mins))
                {
                    rtDW.is_MarketLoginState = IN_day_login;
                    output.status = LOGIN_TIME;
                }
                else
                {
                    if ((strcmp(input.loginTime, "night") == 0) && (input.now_mins == input.night_login_mins))
                    {
                        rtDW.is_MarketLoginState = IN_night_login;
                        output.status = LOGIN_TIME;
                    }
                }
                break;
        }
    }
}

void MarketLoginState::update(MarketLoginState& login_state)
{
    time_t now = {0};
    struct tm *timenow = NULL;

    while(1)
    {
        time(&now);
        timenow = localtime(&now);//获取当前时间
        login_state.input.now_mins= timenow->tm_hour*60 + timenow->tm_min;

        login_state.step();
        sleep(1);
    }
}

void MarketLoginState::initialize()
{
    auto& jsonCfg = utils::JsonConfig::getInstance();

    string modeStr = jsonCfg.getConfig("market","LoginMode").get<std::string>();
    strcpy(input.loginMode, modeStr.c_str());

    string timeStr = jsonCfg.getConfig("market","DayLoginTime").get<std::string>();
    vector<string> timeVec = utils::splitString(timeStr,  ":");
    input.day_login_mins = atoi(timeVec[0].c_str())*60 + atoi(timeVec[1].c_str());

    timeStr = jsonCfg.getConfig("market","DayLogoutTime").get<std::string>();
    timeVec= utils::splitString(timeStr,  ":");
    input.day_logout_mins= atoi(timeVec[0].c_str())*60 + atoi(timeVec[1].c_str());

    timeStr = jsonCfg.getConfig("market","NightLoginTime").get<std::string>();
    timeVec = utils::splitString(timeStr,  ":");
    input.night_login_mins= atoi(timeVec[0].c_str())*60 + atoi(timeVec[1].c_str());

    timeStr = jsonCfg.getConfig("market","NightLogoutTime").get<std::string>();
    timeVec = utils::splitString(timeStr,  ":");
    input.night_logout_mins= atoi(timeVec[0].c_str())*60 + atoi(timeVec[1].c_str());

    timeStr = jsonCfg.getConfig("market","LoginTime").get<std::string>();
    strcpy(input.loginTime, timeStr.c_str());
}

MarketLoginState::MarketLoginState()
{
    initialize();
}

MarketLoginState::~MarketLoginState()
{
    ;
}

ERROR_STATUS * MarketLoginState::getRTM()
{
    return (&rtM);
}
