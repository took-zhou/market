#include "market/domain/components/publish_market_state.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/market-trader.pb.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "common/self/semaphore.h"
#include "market/domain/components/market_time_state.h"
#include "market/domain/market_service.h"
#include "market/infra/recer_sender.h"

#include <unistd.h>
#include <chrono>
#include <thread>

PublishState::PublishState() {
  ;
  ;
}

void PublishState::PublishEvent(void) {
  auto &market_ser = MarketService::GetInstance();
  static SubTimeState prev_sub_time_state = kInInitSts;
  SubTimeState now_sub_time_state = market_ser.ROLE(MarketTimeState).GetSubTimeState();
  if (prev_sub_time_state == kInInitSts) {
    prev_sub_time_state = now_sub_time_state;
  }
  if (prev_sub_time_state != now_sub_time_state && market_ser.login_state == kLoginState) {
    PublishToStrategy();
    prev_sub_time_state = now_sub_time_state;
    wait_publish_count_ = 0;
  }
  if (publish_flag_ == true) {
    wait_publish_count_++;
    if (wait_publish_count_ >= max_wait_pushlish_count_) {
      WARNING_LOG("wait publish market rsp time out");
    }
  }
}

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

  strategy_market::message tick;
  auto market_state = tick.mutable_market_state_req();
  market_state->set_market_state(state);
  market_state->set_date(date_buff);
  market_state->set_is_last(1);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "MarketStateReq";
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).SendMsg(msg);
  SetPublishFlag();
}

void PublishState::PublishEvent(FtpLoginLogoutStruct *login_logout) {
  PublishToTrader(login_logout);
  while (1) {
    if (publish_flag_ == false) {
      wait_publish_count_ = 0;
      break;
    } else if (publish_flag_ == true) {
      wait_publish_count_++;
      if (wait_publish_count_ >= max_wait_pushlish_count_) {
        WARNING_LOG("wait publish market rsp time out");
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  PublishToStrategy(login_logout);
  while (1) {
    if (publish_flag_ == false) {
      wait_publish_count_ = 0;
      break;
    } else if (publish_flag_ == true) {
      wait_publish_count_++;
      if (wait_publish_count_ >= max_wait_pushlish_count_) {
        WARNING_LOG("wait publish market rsp time out");
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void PublishState::PublishToTrader(FtpLoginLogoutStruct *login_logout) {
  market_trader::MarketStateReq_MarketState state = market_trader::MarketStateReq_MarketState_reserve;
  if (strcmp(login_logout->market_state, "day_close") == 0) {
    state = market_trader::MarketStateReq_MarketState_day_close;
    INFO_LOG("Publish makret state: day_close, date: %s to strategy.", login_logout->date);
  } else if (strcmp(login_logout->market_state, "night_close") == 0) {
    state = market_trader::MarketStateReq_MarketState_night_close;
    INFO_LOG("Publish makret state: night_close, date: %s to strategy.", login_logout->date);
  } else if (strcmp(login_logout->market_state, "day_open") == 0) {
    state = market_trader::MarketStateReq_MarketState_day_open;
    INFO_LOG("Publish makret state: day_open, date: %s to strategy.", login_logout->date);
  } else if (strcmp(login_logout->market_state, "night_open") == 0) {
    state = market_trader::MarketStateReq_MarketState_night_open;
    INFO_LOG("Publish makret state: night_open, date: %s to strategy.", login_logout->date);
  }

  market_trader::message tick;
  auto market_state = tick.mutable_market_state_req();
  market_state->set_market_state(state);
  market_state->set_date(login_logout->date);
  market_state->set_is_last((bool)true);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pb_msg);
  msg.session_name = "market_trader";
  msg.msg_name = "MarketStateReq";
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).SendMsg(msg);
  SetPublishFlag();
}

void PublishState::PublishToStrategy(FtpLoginLogoutStruct *login_logout) {
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

  strategy_market::message tick;
  auto market_state = tick.mutable_market_state_req();
  market_state->set_market_state(state);
  market_state->set_date(login_logout->date);
  market_state->set_is_last((bool)true);

  utils::ItpMsg msg;
  tick.SerializeToString(&msg.pb_msg);
  msg.session_name = "strategy_market";
  msg.msg_name = "MarketStateReq";
  auto &recer_sender = RecerSender::GetInstance();
  recer_sender.ROLE(Sender).ROLE(ProxySender).SendMsg(msg);
  SetPublishFlag();
}

void PublishState::ClearPublishFlag() {
  pthread_mutex_lock(&(sm_mutex_));
  publish_flag_ = false;
  pthread_mutex_unlock(&(sm_mutex_));
}

void PublishState::SetPublishFlag() {
  pthread_mutex_lock(&(sm_mutex_));
  publish_flag_ = true;
  pthread_mutex_unlock(&(sm_mutex_));
}

void PublishState::GetTradeData(char *buff) {
  auto &market_ser = MarketService::GetInstance();
  auto timenow = market_ser.ROLE(MarketTimeState).GetTimeNow();
  if (timenow != nullptr) {
    if (19 <= timenow->tm_hour && timenow->tm_hour <= 23) {
      if (timenow->tm_wday == 5) {
        time_t tsecond = mktime(timenow) + 259200;
        strftime(buff, 10, "%Y%m%d", localtime(&tsecond));
      } else {
        time_t tsecond = mktime(timenow) + 86400;
        strftime(buff, 10, "%Y%m%d", localtime(&tsecond));
      }
    } else if (1 <= timenow->tm_hour && timenow->tm_hour <= 3 && timenow->tm_wday == 6) {
      time_t tsecond = mktime(timenow) + 172800;
      strftime(buff, 10, "%Y%m%d", localtime(&tsecond));
    } else {
      strftime(buff, 10, "%Y%m%d", timenow);
    }
  }
}
