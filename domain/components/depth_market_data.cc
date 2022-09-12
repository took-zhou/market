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

MarketData::MarketData() { instrument_exchange_map.clear(); }

bool MarketData::IsValidTickData(CThostFtdcDepthMarketDataField *pD) {
  tm tick_tm;
  bool ret = false;
  char UpdateTime[27];

  utils::Gbk2Utf8(pD->UpdateTime, UpdateTime, sizeof(UpdateTime));
  strptime(UpdateTime, "%H:%M:%S", &tick_tm);

  int tickSecond = tick_tm.tm_hour * 60 * 60 + tick_tm.tm_min * 60 + tick_tm.tm_sec;
  // system time
  time_t now_time = time(NULL);
  // local time
  tm *local_time = localtime(&now_time);
  int nowSecond = local_time->tm_hour * 60 * 60 + local_time->tm_min * 60 + local_time->tm_sec;

  int delaySecond = nowSecond - tickSecond;

#ifdef BENCH_TEST
  if (delaySecond != 0) {
    INFO_LOG("%s local time: %02d:%02d:%02d--ctp time: %s delaySecond %d", pD->InstrumentID, local_time->tm_hour, local_time->tm_min,
             local_time->tm_sec, pD->UpdateTime, delaySecond);
  }
#endif

  if (delaySecond <= 180 && delaySecond >= -180 && (pD->BidPrice1 > 0.0 || pD->AskPrice1 > 0.0)) {
    ret = true;
  }

  return ret;
}

bool MarketData::IsValidTickData(XTPMD *pD) {
  tm tick_tm;
  bool ret = false;

  string data_time = std::to_string(pD->data_time);
  string UpdateTime = data_time.substr(8, 6);
  strptime(UpdateTime.c_str(), "%H%M%S", &tick_tm);

  int tickSecond = tick_tm.tm_hour * 60 * 60 + tick_tm.tm_min * 60 + tick_tm.tm_sec;
  // system time
  time_t now_time = time(NULL);
  // local time
  tm *local_time = localtime(&now_time);
  int nowSecond = local_time->tm_hour * 60 * 60 + local_time->tm_min * 60 + local_time->tm_sec;

  int delaySecond = nowSecond - tickSecond;
#ifdef BENCH_TEST
  if (delaySecond != 0) {
    INFO_LOG("%s local time: %02d:%02d:%02d--ctp time:  %02d:%02d:%02d delaySecond %d", pD->ticker, local_time->tm_hour, local_time->tm_min,
             local_time->tm_sec, tick_tm.tm_hour, tick_tm.tm_min, tick_tm.tm_sec, delaySecond);
  }
#endif
  if (delaySecond <= 180 && delaySecond >= -180 && pD->bid_qty[0] > 0 && pD->ask_qty[0] > 0 && pD->bid[0] > 0.0 && pD->ask[0] > 0.0) {
    ret = true;
  }

  return ret;
}

bool MarketData::InsertInsExchPair(const std::string &ins, const std::string &exch) {
  auto iter = instrument_exchange_map.find(ins);
  if (iter != instrument_exchange_map.end()) {
    if (instrument_exchange_map[ins] != exch) {
      instrument_exchange_map.insert(pair<string, string>(ins, exch));
    }
  } else {
    instrument_exchange_map.insert(pair<string, string>(ins, exch));
  }
  return true;
}

bool MarketData::ClearInsExchPair(void) {
  instrument_exchange_map.clear();
  return true;
}

bool MarketData::ShowInsExchPair(void) { INFO_LOG("instrument_exchange_map size: %d", (int)instrument_exchange_map.size()); }

double MarketData::Max2zero(double num) {
  double re;
  if (num >= 100000000) {
    re = 0;
  } else {
    re = num;
  }
  return re;
}

std::string MarketData::FindExchange(const std::string &ins) {
  auto iter = instrument_exchange_map.find(ins);
  if (iter != instrument_exchange_map.end()) {
    return instrument_exchange_map[ins];
  } else {
    ERROR_LOG("not find ins: %s", ins.c_str());
    return "";
  }
}

bool MarketData::get_local_time(char *t_arr) {
  struct timeval curTime;
  gettimeofday(&curTime, NULL);
  int milli = curTime.tv_usec / 1000;

  char buffer[80] = {0};
  struct tm nowTime;
  localtime_r(&curTime.tv_sec,
              &nowTime);  //把得到的值存入临时分配的内存中，线程安全
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &nowTime);

  sprintf(t_arr, "%s.%03d", buffer, milli);
}

bool MarketData::get_local_time(long &stamp) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  stamp = tv.tv_sec;
}

bool MarketData::get_assembling_time(char *t_arr, CThostFtdcDepthMarketDataField *pD) {
  int y, m, d;
  time_t now_time = time(NULL);
  // local time
  tm *local_time = localtime(&now_time);

  y = 1900 + local_time->tm_year;
  m = 1 + local_time->tm_mon;
  d = local_time->tm_mday;

  sprintf(t_arr, "%04d-%02d-%02d %s.%d", y, m, d, pD->UpdateTime, pD->UpdateMillisec);
}

bool MarketData::get_assembling_time(char *t_arr, XTPMD *pD) {
  string data_time = std::to_string(pD->data_time);
  string y = data_time.substr(0, 4);
  string mon = data_time.substr(4, 2);
  string d = data_time.substr(6, 2);
  string h = data_time.substr(8, 2);
  string min = data_time.substr(10, 2);
  string s = data_time.substr(12, 2);
  string ms = data_time.substr(14, 3);

  sprintf(t_arr, "%s-%s-%s %s:%s:%s.%s", y.c_str(), mon.c_str(), d.c_str(), h.c_str(), min.c_str(), s.c_str(), ms.c_str());
}
