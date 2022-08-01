#include "market/domain/components/publishMarketState.h"
#include "common/extern/log/log.h"
#include "common/self/protobuf/manage-market.pb.h"
#include "common/self/protobuf/strategy-market.pb.h"
#include "market/domain/components/ctpMarketApi/marketTimeState.h"
#include "market/domain/marketService.h"
#include "market/infra/recerSender.h"

#include <unistd.h>

publishState::publishState() {
  ;
  ;
}

void publishState::publish_event(void) {
  publish_to_strategy();
  publish_to_manage();
}

void publishState::publish_to_strategy(void) {
  char date_buff[10];
  get_trade_data(date_buff);
  auto &marketSer = MarketService::getInstance();

  strategy_market::TickMarketState_MarketState state = strategy_market::TickMarketState_MarketState_reserve;
  if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_day_logout) {
    state = strategy_market::TickMarketState_MarketState_day_close;
    INFO_LOG("Publish makret state: day_close, date: %s to strategy.", date_buff);
  } else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_night_logout) {
    state = strategy_market::TickMarketState_MarketState_night_close;
    INFO_LOG("Publish makret state: night_close, date: %s to strategy.", date_buff);
  } else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_day_login) {
    state = strategy_market::TickMarketState_MarketState_day_open;
    INFO_LOG("Publish makret state: day_open, date: %s to strategy.", date_buff);
  } else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_night_login) {
    state = strategy_market::TickMarketState_MarketState_night_open;
    INFO_LOG("Publish makret state: night_open, date: %s to strategy.", date_buff);
  }

  auto keyNameList = marketSer.ROLE(controlPara).getIdentifyList();
  for (auto &keyname : keyNameList) {
    strategy_market::message tick;
    auto market_state = tick.mutable_market_state();

    market_state->set_market_state(state);
    market_state->set_date(date_buff);
    std::string tickStr;
    tick.SerializeToString(&tickStr);
    auto &recerSender = RecerSender::getInstance();
    string topic = "strategy_market.TickMarketState." + keyname;
    recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());
    std::this_thread::sleep_for(10ms);
  }
}

void publishState::publish_to_manage(void) {
  char date_buff[10];
  get_trade_data(date_buff);
  auto &marketSer = MarketService::getInstance();

  manage_market::TickMarketState_MarketState state = manage_market::TickMarketState_MarketState_reserve;
  if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_day_logout) {
    state = manage_market::TickMarketState_MarketState_day_close;
    INFO_LOG("Publish makret state: day_close, date: %s to manage.", date_buff);
  } else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_night_logout) {
    state = manage_market::TickMarketState_MarketState_night_close;
    INFO_LOG("Publish makret state: night_close, date: %s to manage.", date_buff);
  } else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_day_login) {
    state = manage_market::TickMarketState_MarketState_day_open;
    INFO_LOG("Publish makret state: day_open to, date: %s manage.", date_buff);
  } else if (marketSer.ROLE(Market).ROLE(MarketTimeState).rtDW.is_MarketTimeState == IN_night_login) {
    state = manage_market::TickMarketState_MarketState_night_open;
    INFO_LOG("Publish makret state: night_open, date: %s to manage.", date_buff);
  }

  manage_market::message tick;
  auto market_state = tick.mutable_market_state();

  market_state->set_market_state(state);
  market_state->set_date(date_buff);
  std::string tickStr;
  tick.SerializeToString(&tickStr);
  auto &recerSender = RecerSender::getInstance();
  string topic = "manage_market.TickMarketState.00000000000";
  recerSender.ROLE(Sender).ROLE(ProxySender).send(topic.c_str(), tickStr.c_str());
}

int publishState::is_leap_year(int y) {
  if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) {
    return 1;
  } else {
    return 0;
  }
}

void publishState::get_trade_data(char *buff) {
  int y, m, d, a[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  time_t now_time = time(NULL);
  // local time
  tm *local_time = localtime(&now_time);

  y = 1900 + local_time->tm_year;
  m = 1 + local_time->tm_mon;
  d = local_time->tm_mday;

  if (is_leap_year(y) == 1) {
    a[1] = 29;
  }

  if (20 <= local_time->tm_hour && local_time->tm_hour <= 23) {
    if (local_time->tm_wday == 5) {
      d += 3;
      while (d > a[m - 1]) {
        d -= a[m - 1];
        m++;
        if (m > 12) {
          m = 1;
          y++;
          if (is_leap_year(y) == 1) {
            a[1] = 29;
          } else {
            a[1] = 28;
          }
        }
      }
      sprintf(buff, "%04d%02d%02d", y, m, d);
    } else {
      d += 1;
      while (d > a[m - 1]) {
        d -= a[m - 1];
        m++;
        if (m > 12) {
          m = 1;
          y++;
          if (is_leap_year(y) == 1) {
            a[1] = 29;
          } else {
            a[1] = 28;
          }
        }
      }
      sprintf(buff, "%04d%02d%02d", y, m, d);
    }
  } else if (1 <= local_time->tm_hour && local_time->tm_hour <= 3 && local_time->tm_wday == 6) {
    d += 2;
    while (d > a[m - 1]) {
      d -= a[m - 1];
      m++;
      if (m > 12) {
        m = 1;
        y++;
        if (is_leap_year(y) == 1) {
          a[1] = 29;
        } else {
          a[1] = 28;
        }
      }
    }
    sprintf(buff, "%04d%02d%02d", y, m, d);
  } else {
    sprintf(buff, "%04d%02d%02d", y, m, d);
  }
}
