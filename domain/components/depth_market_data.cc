#include "market/domain/components/depth_market_data.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/profiler.h"
#include "common/self/utils.h"
#include "market/domain/market_service.h"

MarketData::MarketData() { instrument_exchange_map_.clear(); }

bool MarketData::IsValidTickData(CThostFtdcDepthMarketDataField *p_d) {
  tm tick_tm;
  bool ret = false;
  auto &market_ser = MarketService::GetInstance();
  auto timenow = market_ser.ROLE(MarketTimeState).GetTimeNow();
  if (timenow != nullptr) {
    strptime(p_d->UpdateTime, "%H:%M:%S", &tick_tm);

    int tick_second = tick_tm.tm_hour * 60 * 60 + tick_tm.tm_min * 60 + tick_tm.tm_sec;
    int now_second = timenow->tm_hour * 60 * 60 + timenow->tm_min * 60 + timenow->tm_sec;

    int delay_second = now_second - tick_second;

#ifdef BENCH_TEST
    if (delay_second != 0) {
      INFO_LOG("%s local time: %02d:%02d:%02d--ctp time: %s delay_second %d", p_d->InstrumentID, local_time->tm_hour, local_time->tm_min,
               local_time->tm_sec, p_d->UpdateTime, delay_second);
    }
#endif

    // INFO_LOG("ins: %s, BidPrice1: %f AskPrice1: %f", p_d->InstrumentID, p_d->BidPrice1, p_d->AskPrice1);
    if (delay_second <= 180 && delay_second >= -180 && (p_d->BidPrice1 > 0.0 || p_d->AskPrice1 > 0.0)) {
      ret = true;
    }
  } else {
    ret = false;
  }

  return ret;
}

bool MarketData::IsValidTickData(XTPMD *p_d) {
  tm tick_tm;
  bool ret = false;
  auto &market_ser = MarketService::GetInstance();
  auto timenow = market_ser.ROLE(MarketTimeState).GetTimeNow();
  if (timenow != nullptr) {
    string data_time = std::to_string(p_d->data_time);
    string update_time = data_time.substr(8, 6);
    strptime(update_time.c_str(), "%H%M%S", &tick_tm);

    int tick_second = tick_tm.tm_hour * 60 * 60 + tick_tm.tm_min * 60 + tick_tm.tm_sec;
    int now_second = timenow->tm_hour * 60 * 60 + timenow->tm_min * 60 + timenow->tm_sec;

    int delay_second = now_second - tick_second;
#ifdef BENCH_TEST
    if (delay_second != 0) {
      INFO_LOG("%s local time: %02d:%02d:%02d--ctp time:  %02d:%02d:%02d delay_second %d", p_d->ticker, local_time->tm_hour,
               local_time->tm_min, local_time->tm_sec, tick_tm.tm_hour, tick_tm.tm_min, tick_tm.tm_sec, delay_second);
    }
#endif
    if (delay_second <= 180 && delay_second >= -180 && p_d->bid_qty[0] > 0 && p_d->ask_qty[0] > 0 && p_d->bid[0] > 0.0 &&
        p_d->ask[0] > 0.0) {
      ret = true;
    }
  } else {
    ret = false;
  }

  return ret;
}

double MarketData::Max2zero(double num) {
  if (num >= 100000000) {
    return 0.0;
  } else {
    return num;
  }
}

bool MarketData::GetAssemblingTime(char *t_arr, CThostFtdcDepthMarketDataField *p_d) {
  int year, month, day;
  auto &market_ser = MarketService::GetInstance();
  auto timenow = market_ser.ROLE(MarketTimeState).GetTimeNow();

  if (timenow != nullptr) {
    year = 1900 + timenow->tm_year;
    month = 1 + timenow->tm_mon;
    day = timenow->tm_mday;

    sprintf(t_arr, "%04d-%02d-%02d %s.%d", year, month, day, p_d->UpdateTime, p_d->UpdateMillisec);
    return true;
  } else {
    return false;
  }
}

bool MarketData::GetAssemblingTime(char *t_arr, XTPMD *p_d) {
  string data_time = std::to_string(p_d->data_time);
  string year = data_time.substr(0, 4);
  string mon = data_time.substr(4, 2);
  string day = data_time.substr(6, 2);
  string hour = data_time.substr(8, 2);
  string min = data_time.substr(10, 2);
  string sec = data_time.substr(12, 2);
  string msec = data_time.substr(14, 3);

  sprintf(t_arr, "%s-%s-%s %s:%s:%s.%s", year.c_str(), mon.c_str(), day.c_str(), hour.c_str(), min.c_str(), sec.c_str(), msec.c_str());

  return true;
}
