/*
 * MarketTimeState.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/domain/components/marketTimeState.h"
#include <string.h>
#include <unistd.h>
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/fileUtil.h"
#include "common/self/protobuf/ctpview-market.pb.h"
#include "common/self/timer.h"
#include "common/self/utils.h"

U32 MarketTimeState::isDuringDayLogoutTime(void) {
  U32 out;
  out = 0U;

  if (((input.day_logout_mins < input.now_mins) && (input.now_mins < input.night_login_mins)) || (time_state == LOGOUT_TIME)) {
    out = 1U;
  }

  return out;
}

U32 MarketTimeState::isDuringNightLogoutTime(void) {
  U32 out;
  out = 0U;

  if (((input.night_logout_mins < input.now_mins) && (input.now_mins < input.day_login_mins)) || (time_state == LOGOUT_TIME)) {
    out = 1U;
  }

  return out;
}

U32 MarketTimeState::isDuringDayLoginTime(void) {
  U32 out;
  out = 0U;

  if ((input.day_login_mins <= input.now_mins) && (input.now_mins <= input.day_logout_mins) && (time_state != LOGOUT_TIME)) {
    out = 1U;
  }

  return out;
}

U32 MarketTimeState::isDuringNightLoginTime(void) {
  U32 out;
  out = 0U;

  if (((input.night_login_mins <= input.now_mins) || (input.now_mins <= input.night_logout_mins)) && (time_state != LOGOUT_TIME)) {
    out = 1U;
  }

  return out;
}

// Model step function
void MarketTimeState::step() {
  if (rtDW.is_active_MarketTimeState == 0U) {
    rtDW.is_active_MarketTimeState = 1U;
    rtDW.is_MarketTimeState = IN_init_sts;
    output.status = LOGOUT_TIME;
  } else {
    switch (rtDW.is_MarketTimeState) {
      case IN_day_login:
        if (isDuringDayLogoutTime()) {
          rtDW.is_MarketTimeState = IN_day_logout;
          output.status = LOGOUT_TIME;
        }
        break;

      case IN_day_logout:
        if (isDuringNightLoginTime()) {
          rtDW.is_MarketTimeState = IN_night_login;
          output.status = LOGIN_TIME;
        } else if (isDuringDayLoginTime()) {
          rtDW.is_MarketTimeState = IN_day_login;
          output.status = LOGIN_TIME;
        }
        break;

      case IN_init_sts:
        if (isDuringDayLogoutTime()) {
          rtDW.is_MarketTimeState = IN_day_logout;
          output.status = LOGOUT_TIME;
        } else if (isDuringNightLogoutTime()) {
          rtDW.is_MarketTimeState = IN_night_logout;
          output.status = LOGOUT_TIME;
        } else if (isDuringDayLoginTime()) {
          rtDW.is_MarketTimeState = IN_day_login;
          output.status = LOGIN_TIME;
        } else if (isDuringNightLoginTime()) {
          rtDW.is_MarketTimeState = IN_night_login;
          output.status = LOGIN_TIME;
        }
        break;

      case IN_night_login:
        if (isDuringNightLogoutTime()) {
          rtDW.is_MarketTimeState = IN_night_logout;
          output.status = LOGOUT_TIME;
        }
        break;

      case IN_night_logout:
        if (isDuringDayLoginTime()) {
          rtDW.is_MarketTimeState = IN_day_login;
          output.status = LOGIN_TIME;
        } else if (isDuringNightLoginTime()) {
          rtDW.is_MarketTimeState = IN_night_login;
          output.status = LOGIN_TIME;
        }
        break;

      default:
        break;
    }
  }
}

void MarketTimeState::update(void) {
  time_t now = {0};
  struct tm *timenow = NULL;

  while (1) {
    time(&now);
    timenow = localtime(&now);  //获取当前时间
    input.now_mins = timenow->tm_hour * 60 + timenow->tm_min;

    step();

    std::this_thread::sleep_for(1000ms);
  }
}

void MarketTimeState::set_time_state(int command) {
  if (command == ctpview_market::LoginControl_Command_login) {
    time_state = LOGIN_TIME;
  } else if (command == ctpview_market::LoginControl_Command_logout) {
    time_state = LOGOUT_TIME;
  } else if (command == ctpview_market::LoginControl_Command_reserve) {
    time_state = RESERVE;
  }
}

MarketTimeState::MarketTimeState() {
  auto &jsonCfg = utils::JsonConfig::getInstance();

  string timeStr = jsonCfg.getConfig("market", "LogInTimeList").get<std::string>();
  vector<string> timeDurationSplited = utils::splitString(timeStr, ";");
  vector<string> dayTimeStrSplit = utils::splitString(timeDurationSplited[0], "-");
  vector<string> nightTimeStrSplit = utils::splitString(timeDurationSplited[1], "-");
  vector<string> dayStartStrSplit = utils::splitString(dayTimeStrSplit[0], ":");
  vector<string> dayEndStrSplit = utils::splitString(dayTimeStrSplit[1], ":");
  vector<string> nightStartStrSplit = utils::splitString(nightTimeStrSplit[0], ":");
  vector<string> nightEndStrSplit = utils::splitString(nightTimeStrSplit[1], ":");

  input.day_login_mins = atoi(dayStartStrSplit[0].c_str()) * 60 + atoi(dayStartStrSplit[1].c_str());
  input.day_logout_mins = atoi(dayEndStrSplit[0].c_str()) * 60 + atoi(dayEndStrSplit[1].c_str());
  input.night_login_mins = atoi(nightStartStrSplit[0].c_str()) * 60 + atoi(nightStartStrSplit[1].c_str());
  input.night_logout_mins = atoi(nightEndStrSplit[0].c_str()) * 60 + atoi(nightEndStrSplit[1].c_str());

  INFO_LOG("%d %d %d %d", input.day_login_mins, input.day_logout_mins, input.night_login_mins, input.night_logout_mins);
}

MarketTimeState::~MarketTimeState() { ; }

ERROR_STATUS *MarketTimeState::getRTM() { return (&rtM); }
