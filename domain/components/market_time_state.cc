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
#include "common/self/utils.h"

uint8_t MarketTimeState::IsDuringDayLogoutTime(void) {
  uint8_t out;
  out = 0U;

  if (day_login_mins_ <= day_logout_mins_) {
    if (now_mins_ < day_login_mins_ || now_mins_ > day_logout_mins_) {
      out = 1U;
    }
  } else {
    if (now_mins_ < day_login_mins_ && now_mins_ > day_logout_mins_) {
      out = 1U;
    }
  }

  return out;
}

uint8_t MarketTimeState::IsDuringNightLogoutTime(void) {
  uint8_t out;
  out = 0U;

  if (night_login_mins_ != -1 && night_logout_mins_ != -1) {
    if (night_login_mins_ <= night_logout_mins_) {
      if (now_mins_ < night_login_mins_ || now_mins_ > night_logout_mins_) {
        out = 1U;
      }
    } else {
      if (now_mins_ < night_login_mins_ && now_mins_ > night_logout_mins_) {
        out = 1U;
      }
    }
  }

  return out;
}

uint8_t MarketTimeState::IsDuringDayLoginTime(void) {
  uint8_t out;
  out = 0U;

  if (day_login_mins_ <= day_logout_mins_) {
    if (now_mins_ >= day_login_mins_ && now_mins_ <= day_logout_mins_) {
      out = 1U;
    }
  } else {
    if (now_mins_ >= day_login_mins_ || now_mins_ <= day_logout_mins_) {
      out = 1U;
    }
  }

  return out;
}

uint8_t MarketTimeState::IsDuringNightLoginTime(void) {
  uint8_t out;
  out = 0U;

  if (night_login_mins_ != -1 && night_logout_mins_ != -1) {
    if (night_login_mins_ <= night_logout_mins_) {
      if (now_mins_ >= night_login_mins_ && now_mins_ <= night_logout_mins_) {
        out = 1U;
      }
    } else {
      if (now_mins_ >= night_login_mins_ || now_mins_ <= night_logout_mins_) {
        out = 1U;
      }
    }
  }

  return out;
}

void MarketTimeState::Step() {
  if (is_active_step_ == 0U) {
    is_active_step_ = 1U;
    sub_time_state_ = kInInitSts;
    time_state_ = kLogoutTime;
  } else {
    switch (sub_time_state_) {
      case kInDayLogin:
        if (IsDuringDayLogoutTime()) {
          sub_time_state_ = kInDayLogout;
          time_state_ = kLogoutTime;
        }
        break;

      case kInDayLogout:
        if (IsDuringNightLoginTime()) {
          sub_time_state_ = kInNightLogin;
          time_state_ = kLoginTime;
        } else if (IsDuringDayLoginTime()) {
          sub_time_state_ = kInDayLogin;
          time_state_ = kLoginTime;
        }
        break;

      case kInInitSts:
        if (IsDuringDayLogoutTime() && !IsDuringNightLoginTime()) {
          sub_time_state_ = kInDayLogout;
          time_state_ = kLogoutTime;
        } else if (IsDuringNightLogoutTime() && !IsDuringDayLoginTime()) {
          sub_time_state_ = kInNightLogout;
          time_state_ = kLogoutTime;
        } else if (IsDuringDayLoginTime()) {
          sub_time_state_ = kInDayLogin;
          time_state_ = kLoginTime;
        } else if (IsDuringNightLoginTime()) {
          sub_time_state_ = kInNightLogin;
          time_state_ = kLoginTime;
        }
        break;

      case kInNightLogin:
        if (IsDuringNightLogoutTime()) {
          sub_time_state_ = kInNightLogout;
          time_state_ = kLogoutTime;
        }
        break;

      case kInNightLogout:
        if (IsDuringDayLoginTime()) {
          sub_time_state_ = kInDayLogin;
          time_state_ = kLoginTime;
        } else if (IsDuringNightLoginTime()) {
          sub_time_state_ = kInNightLogin;
          time_state_ = kLoginTime;
        }
        break;

      default:
        break;
    }
  }
}

void MarketTimeState::Update(void) {
  time_t now = {0};

  time(&now);
  timenow_ = localtime(&now);  // 获取当前时间
  now_mins_ = timenow_->tm_hour * 60 + timenow_->tm_min;

  Step();
}

void MarketTimeState::Simulate(const char *time_str) {
  static struct tm tm_temp;
  if (strptime(time_str, "%Y-%m-%d %H:%M:%S", &tm_temp) != NULL) {
    timenow_ = &tm_temp;
    now_mins_ = timenow_->tm_hour * 60 + timenow_->tm_min;
  }

  Step();
}

void MarketTimeState::SetTimeState(int command) {
  if (command == ctpview_market::LoginControl_Command_login) {
    debug_time_state_ = kLoginTime;
  } else if (command == ctpview_market::LoginControl_Command_logout) {
    debug_time_state_ = kLogoutTime;
  } else if (command == ctpview_market::LoginControl_Command_reserve) {
    debug_time_state_ = kReserve;
  }
}

void MarketTimeState::SetSubTimeState(int command) {
  if (command == ctpview_market::LoginControl_Command_login) {
    debug_sub_time_state_ = kInDayLogin;
  } else if (command == ctpview_market::LoginControl_Command_logout) {
    debug_sub_time_state_ = kInDayLogout;
  } else if (command == ctpview_market::LoginControl_Command_reserve) {
    debug_sub_time_state_ = kInInitSts;
  }
}

MarketTimeState::MarketTimeState() {
  auto &json_cfg = utils::JsonConfig::GetInstance();

  auto time_str = json_cfg.GetConfig("market", "LogInTimeList").get<std::string>();
  vector<string> time_duration_splited = utils::SplitString(time_str, ";");
  if (time_duration_splited.size() == 1) {
    vector<string> day_time_str_split = utils::SplitString(time_duration_splited[0], "-");
    vector<string> day_start_str_split = utils::SplitString(day_time_str_split[0], ":");
    vector<string> day_end_str_split = utils::SplitString(day_time_str_split[1], ":");
    day_login_mins_ = stoi(day_start_str_split[0]) * 60 + stoi(day_start_str_split[1]);
    day_logout_mins_ = stoi(day_end_str_split[0]) * 60 + stoi(day_end_str_split[1]);
    night_login_mins_ = -1;
    night_logout_mins_ = -1;
  } else if (time_duration_splited.size() == 2) {
    vector<string> day_time_str_split = utils::SplitString(time_duration_splited[0], "-");
    vector<string> night_time_str_split = utils::SplitString(time_duration_splited[1], "-");
    vector<string> day_start_str_split = utils::SplitString(day_time_str_split[0], ":");
    vector<string> day_end_str_split = utils::SplitString(day_time_str_split[1], ":");
    vector<string> night_start_str_split = utils::SplitString(night_time_str_split[0], ":");
    vector<string> night_end_str_split = utils::SplitString(night_time_str_split[1], ":");
    day_login_mins_ = stoi(day_start_str_split[0]) * 60 + stoi(day_start_str_split[1]);
    day_logout_mins_ = stoi(day_end_str_split[0]) * 60 + stoi(day_end_str_split[1]);
    night_login_mins_ = stoi(night_start_str_split[0]) * 60 + stoi(night_start_str_split[1]);
    night_logout_mins_ = stoi(night_end_str_split[0]) * 60 + stoi(night_end_str_split[1]);
  } else {
    ERROR_LOG("time string: %s is error.", time_str.c_str());
  }

  INFO_LOG("%d %d %d %d", day_login_mins_, day_logout_mins_, night_login_mins_, night_logout_mins_);
}

MarketTimeState::~MarketTimeState() { ; }

TimeState MarketTimeState::GetTimeState() {
  if (debug_time_state_ != kReserve) {
    return debug_time_state_;
  } else {
    return time_state_;
  }
}

SubTimeState MarketTimeState::GetSubTimeState() {
  if (debug_sub_time_state_ != kInInitSts) {
    return debug_sub_time_state_;
  } else {
    return sub_time_state_;
  }
}

struct tm *MarketTimeState::GetTimeNow() {
  return timenow_;
}