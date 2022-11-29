#include "market/domain/components/publish_market_state.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/semaphore.h"
#include "market/domain/components/market_time_state.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"

#include <unistd.h>
#include <thread>

PublishState::PublishState() {
  ;
  ;
}

void PublishState::PublishEvent(void) { PublishToStrategy(); }

void PublishState::PublishToStrategy(void) {
  char date_buff[10];
  GetTradeData(date_buff);
  auto &market_ser = MarketService::GetInstance();

  strategy_market::MarketStateReq_MarketState state = strategy_market::MarketStateReq_MarketState_reserve;
  if (market_ser.ROLE(MarketTimeState).GetSubTimeState() == kInDayLogout) {
    state = strategy_market::MarketStateReq_MarketState_day_close;
    INFO_LOG("Publish makret state: day_close, date: %s to strategy.", date_buff);
  } else if (market_ser.ROLE(MarketTimeState).GetSubTimeState() == kInNightLogout) {
    state = strategy_market::MarketStateReq_MarketState_night_close;
    INFO_LOG("Publish makret state: night_close, date: %s to strategy.", date_buff);
  } else if (market_ser.ROLE(MarketTimeState).GetSubTimeState() == kInDayLogin) {
    state = strategy_market::MarketStateReq_MarketState_day_open;
    INFO_LOG("Publish makret state: day_open, date: %s to strategy.", date_buff);
  } else if (market_ser.ROLE(MarketTimeState).GetSubTimeState() == kInNightLogin) {
    state = strategy_market::MarketStateReq_MarketState_night_open;
    INFO_LOG("Publish makret state: night_open, date: %s to strategy.", date_buff);
  }

  auto key_name_kist = market_ser.ROLE(ControlPara).GetPridList();
  for (auto &keyname : key_name_kist) {
    strategy_market::message tick;
    auto market_state = tick.mutable_market_state_req();

    market_state->set_market_state(state);
    market_state->set_date(date_buff);
    if (key_name_kist.find(keyname) == key_name_kist.end()) {
      market_state->set_is_last(1);
    } else {
      market_state->set_is_last(0);
    }

    utils::ItpMsg msg;
    tick.SerializeToString(&msg.pb_msg);
    msg.session_name = "strategy_market";
    msg.msg_name = "MarketStateReq." + keyname;
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);
  }
}

int PublishState::IsLeapYear(int year) {
  if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
    return 1;
  } else {
    return 0;
  }
}

void PublishState::PublishEvent(BtpLoginLogoutStruct *login_logout) { PublishToStrategy(login_logout); }

void PublishState::PublishToStrategy(BtpLoginLogoutStruct *login_logout) {
  auto &market_ser = MarketService::GetInstance();

  strategy_market::MarketStateReq_MarketState state = strategy_market::MarketStateReq_MarketState_reserve;
  if (strcmp(login_logout->market_state, "day_close") == 0) {
    state = strategy_market::MarketStateReq_MarketState_day_close;
    INFO_LOG("Publish makret state: day_close, date: %s to strategy.", login_logout->date);
  } else if (strcmp(login_logout->market_state, "night_close") == 0) {
    state = strategy_market::MarketStateReq_MarketState_night_close;
    INFO_LOG("Publish makret state: night_close, date: %s to strategy.", login_logout->date);
  } else if (strcmp(login_logout->market_state, "day_open") == 0) {
    state = strategy_market::MarketStateReq_MarketState_day_open;
    INFO_LOG("Publish makret state: day_open, date: %s to strategy.", login_logout->date);
  } else if (strcmp(login_logout->market_state, "night_open") == 0) {
    state = strategy_market::MarketStateReq_MarketState_night_open;
    INFO_LOG("Publish makret state: night_open, date: %s to strategy.", login_logout->date);
  }

  if (login_logout->prid == 0) {
    auto key_name_kist = market_ser.ROLE(ControlPara).GetPridList();
    for (auto &keyname : key_name_kist) {
      strategy_market::message tick;
      auto market_state = tick.mutable_market_state_req();

      market_state->set_market_state(state);
      market_state->set_date(login_logout->date);
      if (key_name_kist.find(keyname) == key_name_kist.end()) {
        market_state->set_is_last(1);
      } else {
        market_state->set_is_last(0);
      }

      utils::ItpMsg msg;
      tick.SerializeToString(&msg.pb_msg);
      msg.session_name = "strategy_market";
      msg.msg_name = "MarketStateReq." + keyname;
      auto &recer_sender = RecerSender::GetInstance();
      recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);
    }
  } else {
    strategy_market::message tick;
    auto market_state = tick.mutable_market_state_req();

    market_state->set_market_state(state);
    market_state->set_date(login_logout->date);
    market_state->set_is_last(1);

    utils::ItpMsg msg;
    tick.SerializeToString(&msg.pb_msg);
    msg.session_name = "strategy_market";
    msg.msg_name = "MarketStateReq." + to_string(login_logout->prid);
    auto &recer_sender = RecerSender::GetInstance();
    recer_sender.ROLE(Sender).ROLE(ProxySender).Send(msg);
  }
}

void PublishState::GetTradeData(char *buff) {
  int year, mon, day, mon_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  auto &market_ser = MarketService::GetInstance();
  auto timenow = market_ser.ROLE(MarketTimeState).GetTimeNow();
  if (timenow != nullptr) {
    year = 1900 + timenow->tm_year;
    mon = 1 + timenow->tm_mon;
    day = timenow->tm_mday;

    if (IsLeapYear(year) == 1) {
      mon_days[1] = 29;
    }

    if (20 <= timenow->tm_hour && timenow->tm_hour <= 23) {
      if (timenow->tm_wday == 5) {
        day += 3;
        while (day > mon_days[mon - 1]) {
          day -= mon_days[mon - 1];
          mon++;
          if (mon > 12) {
            mon = 1;
            year++;
            if (IsLeapYear(year) == 1) {
              mon_days[1] = 29;
            } else {
              mon_days[1] = 28;
            }
          }
        }
        sprintf(buff, "%04d%02d%02d", year, mon, day);
      } else {
        day += 1;
        while (day > mon_days[mon - 1]) {
          day -= mon_days[mon - 1];
          mon++;
          if (mon > 12) {
            mon = 1;
            year++;
            if (IsLeapYear(year) == 1) {
              mon_days[1] = 29;
            } else {
              mon_days[1] = 28;
            }
          }
        }
        sprintf(buff, "%04d%02d%02d", year, mon, day);
      }
    } else if (1 <= timenow->tm_hour && timenow->tm_hour <= 3 && timenow->tm_wday == 6) {
      day += 2;
      while (day > mon_days[mon - 1]) {
        day -= mon_days[mon - 1];
        mon++;
        if (mon > 12) {
          mon = 1;
          year++;
          if (IsLeapYear(year) == 1) {
            mon_days[1] = 29;
          } else {
            mon_days[1] = 28;
          }
        }
      }
      sprintf(buff, "%04d%02d%02d", year, mon, day);
    } else {
      sprintf(buff, "%04d%02d%02d", year, mon, day);
    }
  }
}
