/*
 * MarketTimeState.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */
#include "market/domain/components/market_time_state.h"
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/protobuf/ctpview-market.pb.h"
#include "common/self/timer.h"
#include "common/self/utils.h"

uint8_t MarketTimeState::IsDuringDayLogoutTime(void) {
  uint8_t out;
  out = 0U;

  if (((day_logout_mins < now_mins) && (now_mins < night_login_mins)) || (debug_time_state == kLogoutTime)) {
    out = 1U;
  }

  return out;
}

uint8_t MarketTimeState::IsDuringNightLogoutTime(void) {
  uint8_t out;
  out = 0U;

  if (((night_logout_mins < now_mins) && (now_mins < day_login_mins)) || (debug_time_state == kLogoutTime)) {
    out = 1U;
  }

  return out;
}

uint8_t MarketTimeState::IsDuringDayLoginTime(void) {
  uint8_t out;
  out = 0U;

  if ((day_login_mins <= now_mins) && (now_mins <= day_logout_mins) && (debug_time_state != kLogoutTime)) {
    out = 1U;
  }

  return out;
}

uint8_t MarketTimeState::IsDuringNightLoginTime(void) {
  uint8_t out;
  out = 0U;

  if (((night_login_mins <= now_mins) || (now_mins <= night_logout_mins)) && (debug_time_state != kLogoutTime)) {
    out = 1U;
  }

  return out;
}

void MarketTimeState::Step() {
  if (is_active_step == 0U) {
    is_active_step = 1U;
    sub_time_state = kInInitSts;
    time_state = kLogoutTime;
  } else {
    switch (sub_time_state) {
      case kInDayLogin:
        if (IsDuringDayLogoutTime()) {
          sub_time_state = kInDayLogout;
          time_state = kLogoutTime;
        }
        break;

      case kInDayLogout:
        if (IsDuringNightLoginTime()) {
          sub_time_state = kInNightLogin;
          time_state = kLoginTime;
        } else if (IsDuringDayLoginTime()) {
          sub_time_state = kInDayLogin;
          time_state = kLoginTime;
        }
        break;

      case kInInitSts:
        if (IsDuringDayLogoutTime()) {
          sub_time_state = kInDayLogout;
          time_state = kLogoutTime;
        } else if (IsDuringNightLogoutTime()) {
          sub_time_state = kInNightLogout;
          time_state = kLogoutTime;
        } else if (IsDuringDayLoginTime()) {
          sub_time_state = kInDayLogin;
          time_state = kLoginTime;
        } else if (IsDuringNightLoginTime()) {
          sub_time_state = kInNightLogin;
          time_state = kLoginTime;
        }
        break;

      case kInNightLogin:
        if (IsDuringNightLogoutTime()) {
          sub_time_state = kInNightLogout;
          time_state = kLogoutTime;
        }
        break;

      case kInNightLogout:
        if (IsDuringDayLoginTime()) {
          sub_time_state = kInDayLogin;
          time_state = kLoginTime;
        } else if (IsDuringNightLoginTime()) {
          sub_time_state = kInNightLogin;
          time_state = kLoginTime;
        }
        break;

      default:
        break;
    }
  }
}

void MarketTimeState::Update(void) {
  time_t now = {0};
  struct tm *timenow = NULL;

  while (1) {
    time(&now);
    timenow = localtime(&now);  //获取当前时间
    now_mins = timenow->tm_hour * 60 + timenow->tm_min;

    Step();

    std::this_thread::sleep_for(1000ms);
  }
}

void MarketTimeState::set_time_state(int command) {
  if (command == ctpview_market::LoginControl_Command_login) {
    time_state = kLoginTime;
  } else if (command == ctpview_market::LoginControl_Command_logout) {
    time_state = kLogoutTime;
  } else if (command == ctpview_market::LoginControl_Command_reserve) {
    time_state = kReserve;
  }
}

MarketTimeState::MarketTimeState() {
  auto &jsonCfg = utils::JsonConfig::getInstance();

  string timeStr = jsonCfg.get_config("market", "LogInTimeList").get<std::string>();
  vector<string> timeDurationSplited = utils::SplitString(timeStr, ";");
  vector<string> dayTimeStrSplit = utils::SplitString(timeDurationSplited[0], "-");
  vector<string> nightTimeStrSplit = utils::SplitString(timeDurationSplited[1], "-");
  vector<string> dayStartStrSplit = utils::SplitString(dayTimeStrSplit[0], ":");
  vector<string> dayEndStrSplit = utils::SplitString(dayTimeStrSplit[1], ":");
  vector<string> nightStartStrSplit = utils::SplitString(nightTimeStrSplit[0], ":");
  vector<string> nightEndStrSplit = utils::SplitString(nightTimeStrSplit[1], ":");

  day_login_mins = atoi(dayStartStrSplit[0].c_str()) * 60 + atoi(dayStartStrSplit[1].c_str());
  day_logout_mins = atoi(dayEndStrSplit[0].c_str()) * 60 + atoi(dayEndStrSplit[1].c_str());
  night_login_mins = atoi(nightStartStrSplit[0].c_str()) * 60 + atoi(nightStartStrSplit[1].c_str());
  night_logout_mins = atoi(nightEndStrSplit[0].c_str()) * 60 + atoi(nightEndStrSplit[1].c_str());

  INFO_LOG("%d %d %d %d", day_login_mins, day_logout_mins, night_login_mins, night_logout_mins);
}

MarketTimeState::~MarketTimeState() { ; }

TimeState MarketTimeState::get_time_state() { return time_state; }

SubTimeState MarketTimeState::get_sub_time_state() { return sub_time_state; }