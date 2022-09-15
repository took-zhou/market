// 线程控制相关
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// 定时器相关
#include <signal.h>
#include <sys/time.h>

// 实时时间获取
#include <stddef.h>
#include <time.h>

// 文件夹及文件操作相关
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <vector>

//自定义头文件
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"
#include "market/domain/components/depth_market_data.h"

MarketData::MarketData() { instrument_exchange_map_.clear(); }

bool MarketData::IsValidTickData(CThostFtdcDepthMarketDataField *p_d) {
  tm tick_tm;
  bool ret = false;
  char update_time[27];

  utils::Gbk2Utf8(p_d->UpdateTime, update_time, sizeof(update_time));
  strptime(update_time, "%H:%M:%S", &tick_tm);

  int tick_second = tick_tm.tm_hour * 60 * 60 + tick_tm.tm_min * 60 + tick_tm.tm_sec;
  // system time
  time_t now_time = time(NULL);
  // local time
  tm *local_time = localtime(&now_time);
  int now_second = local_time->tm_hour * 60 * 60 + local_time->tm_min * 60 + local_time->tm_sec;

  int delay_second = now_second - tick_second;

#ifdef BENCH_TEST
  if (delaySecond != 0) {
    INFO_LOG("%s local time: %02d:%02d:%02d--ctp time: %s delaySecond %d", pD->InstrumentID, local_time->tm_hour, local_time->tm_min,
             local_time->tm_sec, pD->UpdateTime, delaySecond);
  }
#endif

  if (delay_second <= 180 && delay_second >= -180 && (p_d->BidPrice1 > 0.0 || p_d->AskPrice1 > 0.0)) {
    ret = true;
  }

  return ret;
}

bool MarketData::IsValidTickData(XTPMD *p_d) {
  tm tick_tm;
  bool ret = false;

  string data_time = std::to_string(p_d->data_time);
  string update_time = data_time.substr(8, 6);
  strptime(update_time.c_str(), "%H%M%S", &tick_tm);

  int tick_second = tick_tm.tm_hour * 60 * 60 + tick_tm.tm_min * 60 + tick_tm.tm_sec;
  // system time
  time_t now_time = time(NULL);
  // local time
  tm *local_time = localtime(&now_time);
  int now_second = local_time->tm_hour * 60 * 60 + local_time->tm_min * 60 + local_time->tm_sec;

  int delay_second = now_second - tick_second;
#ifdef BENCH_TEST
  if (delaySecond != 0) {
    INFO_LOG("%s local time: %02d:%02d:%02d--ctp time:  %02d:%02d:%02d delaySecond %d", pD->ticker, local_time->tm_hour, local_time->tm_min,
             local_time->tm_sec, tick_tm.tm_hour, tick_tm.tm_min, tick_tm.tm_sec, delaySecond);
  }
#endif
  if (delay_second <= 180 && delay_second >= -180 && p_d->bid_qty[0] > 0 && p_d->ask_qty[0] > 0 && p_d->bid[0] > 0.0 && p_d->ask[0] > 0.0) {
    ret = true;
  }

  return ret;
}

bool MarketData::InsertInsExchPair(const std::string &ins, const std::string &exch) {
  auto iter = instrument_exchange_map_.find(ins);
  if (iter != instrument_exchange_map_.end()) {
    if (instrument_exchange_map_[ins] != exch) {
      instrument_exchange_map_.insert(pair<string, string>(ins, exch));
    }
  } else {
    instrument_exchange_map_.insert(pair<string, string>(ins, exch));
  }
  return true;
}

bool MarketData::ClearInsExchPair(void) {
  instrument_exchange_map_.clear();
  return true;
}

bool MarketData::ShowInsExchPair(void) {
  INFO_LOG("instrument_exchange_map size: %d", (int)instrument_exchange_map_.size());
  return true;
}

double MarketData::Max2zero(double num) {
  double ret;
  if (num >= 100000000) {
    ret = 0;
  } else {
    ret = num;
  }
  return ret;
}

std::string MarketData::FindExchange(const std::string &ins) {
  auto iter = instrument_exchange_map_.find(ins);
  if (iter != instrument_exchange_map_.end()) {
    return instrument_exchange_map_[ins];
  } else {
    ERROR_LOG("not find ins: %s", ins.c_str());
    return "";
  }
}

bool MarketData::GetLocalTime(char *t_arr) {
  struct timeval cur_time;
  gettimeofday(&cur_time, NULL);
  int milli = cur_time.tv_usec / 1000;

  char buffer[80] = {0};
  struct tm now_time;
  localtime_r(&cur_time.tv_sec,
              &now_time);  //把得到的值存入临时分配的内存中，线程安全
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &now_time);

  sprintf(t_arr, "%s.%03d", buffer, milli);
  return true;
}

bool MarketData::GetLocalTime(long &stamp) {
  struct timeval time_value;
  gettimeofday(&time_value, NULL);
  stamp = time_value.tv_sec;
  return true;
}

bool MarketData::GetAssemblingTime(char *t_arr, CThostFtdcDepthMarketDataField *p_d) {
  int year, month, day;
  time_t now_time = time(NULL);
  // local time
  tm *local_time = localtime(&now_time);

  year = 1900 + local_time->tm_year;
  month = 1 + local_time->tm_mon;
  day = local_time->tm_mday;

  sprintf(t_arr, "%04d-%02d-%02d %s.%d", year, month, day, p_d->UpdateTime, p_d->UpdateMillisec);
  return true;
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
