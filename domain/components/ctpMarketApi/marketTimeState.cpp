/*
 * MarketTimeState.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/domain/components/ctpMarketApi/marketTimeState.h"
#include "common/self/protobuf/ctpview-market.pb.h"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"
#include "common/extern/log/log.h"
#include <string>
#include <string.h>
#include <unistd.h>

U32 MarketTimeState::isDuringDayLogoutTime(void)
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

U32 MarketTimeState::isDuringNightLogoutTime(void)
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

U32 MarketTimeState::isDuringDayLoginTime(void)
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

U32 MarketTimeState::isDuringNightLoginTime(void)
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

void MarketTimeState::determineLoginMode(void)
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
void MarketTimeState::step()
{
    if (rtDW.is_active_MarketTimeState == 0U) 
    {
        rtDW.is_active_MarketTimeState = 1U;
        rtDW.is_MarketTimeState = IN_init_sts;
        determineLoginMode();
    }
    else
    {
        switch (rtDW.is_MarketTimeState)
        {
            case IN_day_login:
                if (input.now_mins == input.day_logout_mins)
                {
                    rtDW.is_MarketTimeState = IN_day_logout;
                    output.status = LOGOUT_TIME;
                }
                break;

            case IN_day_logout:
                if ((strcmp(input.loginTime, "day") != 0) && (input.now_mins == input.night_login_mins))
                {
                    rtDW.is_MarketTimeState = IN_night_login;
                    output.status = LOGIN_TIME;
                }
                else
                {
                    if ((strcmp(input.loginTime, "day") == 0) && (input.now_mins == input.day_login_mins))
                    {
                        rtDW.is_MarketTimeState = IN_day_login;
                        output.status = LOGIN_TIME;
                    }
                }
                break;

            case IN_init_sts:
                if (isDuringDayLogoutTime() != 0U)
                {
                    rtDW.is_MarketTimeState = IN_day_logout;
                    output.status = LOGOUT_TIME;
                } 
                else if (isDuringNightLogoutTime() != 0U)
                {
                    rtDW.is_MarketTimeState = IN_night_logout;
                    output.status = LOGOUT_TIME;
                }
                else if (isDuringDayLoginTime() != 0U)
                {
                    rtDW.is_MarketTimeState = IN_day_login;
                    output.status = LOGIN_TIME;
                }
                else
                {
                    if (isDuringNightLoginTime() != 0U)
                    {
                        rtDW.is_MarketTimeState = IN_night_login;
                        output.status = LOGIN_TIME;
                    }
                }
                break;

            case IN_night_login:
                if (input.now_mins == input.night_logout_mins)
                {
                    rtDW.is_MarketTimeState = IN_night_logout;
                    output.status = LOGOUT_TIME;
                }
                break;

            default:
                if ((strcmp(input.loginTime, "night") != 0) && (input.now_mins == input.day_login_mins))
                {
                    rtDW.is_MarketTimeState = IN_day_login;
                    output.status = LOGIN_TIME;
                }
                else
                {
                    if ((strcmp(input.loginTime, "night") == 0) && (input.now_mins == input.night_login_mins))
                    {
                        rtDW.is_MarketTimeState = IN_night_login;
                        output.status = LOGIN_TIME;
                    }
                }
                break;
        }
    }
}

void MarketTimeState::update(void)
{
    time_t now = {0};
    struct tm *timenow = NULL;

    while(1)
    {
        time(&now);
        timenow = localtime(&now);//获取当前时间
        input.now_mins= timenow->tm_hour*60 + timenow->tm_min;

        step();

        if (time_state != RESERVE)
        {
            output.status = time_state;
        }
        sleep(1);
    }
}

void MarketTimeState::set_time_state(int command)
{
    if (command == ctpview_market::LoginControl_Command_login)
    {
        time_state = LOGIN_TIME;
    }
    else if (command == ctpview_market::LoginControl_Command_logout)
    {
        time_state = LOGOUT_TIME;
    }
    else if (command == ctpview_market::LoginControl_Command_reserve)
    {
        rtDW.is_active_MarketTimeState = 0U;
        time_state = RESERVE;
    }
}

void MarketTimeState::initialize()
{
    auto& jsonCfg = utils::JsonConfig::getInstance();

    string modeStr = jsonCfg.getConfig("market","LoginMode").get<std::string>();
    strcpy(input.loginMode, modeStr.c_str());

    string timeStr = jsonCfg.getConfig("market","LogInTimeList").get<std::string>();
    vector<string> timeDurationSplited = utils::splitString(timeStr, ";");
    vector<string> dayTimeStrSplit = utils::splitString(timeDurationSplited[0], "-");
    vector<string> nightTimeStrSplit = utils::splitString(timeDurationSplited[1], "-");
    vector<string> dayStartStrSplit = utils::splitString(dayTimeStrSplit[0], ":");
    vector<string> dayEndStrSplit = utils::splitString(dayTimeStrSplit[1], ":");
    vector<string> nightStartStrSplit = utils::splitString(nightTimeStrSplit[0], ":");
    vector<string> nightEndStrSplit = utils::splitString(nightTimeStrSplit[1], ":");

    input.day_login_mins = atoi(dayStartStrSplit[0].c_str())*60 + atoi(dayStartStrSplit[1].c_str());
    input.day_logout_mins = atoi(dayEndStrSplit[0].c_str())*60 + atoi(dayEndStrSplit[1].c_str());
    input.night_login_mins = atoi(nightStartStrSplit[0].c_str())*60 + atoi(nightStartStrSplit[1].c_str());
    input.night_logout_mins = atoi(nightEndStrSplit[0].c_str())*60 + atoi(nightEndStrSplit[1].c_str());

    INFO_LOG("%d %d %d %d", input.day_login_mins, input.day_logout_mins, input.night_login_mins, input.night_logout_mins);
    timeStr = jsonCfg.getConfig("market","LoginTime").get<std::string>();
    strcpy(input.loginTime, timeStr.c_str());
}

MarketTimeState::MarketTimeState()
{
    initialize();
}

MarketTimeState::~MarketTimeState()
{
    ;
}

ERROR_STATUS * MarketTimeState::getRTM()
{
    return (&rtM);
}
