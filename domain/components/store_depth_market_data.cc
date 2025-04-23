#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <string>

#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"
#include "market/domain/components/store_depth_market_data.h"
#include "market/domain/market_service.h"

LoadData::LoadData() {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  history_tick_folder_ = json_cfg.GetConfig("market", "HistoryTickPath").get<std::string>();
}

void LoadData::FormDepthMarketData2Stringflow(CThostFtdcDepthMarketDataField *p_d) {
  /// 合约代码 TThostFtdcInstrumentIDType char[31]
  char instrument_id[93];
  utils::Gbk2Utf8(p_d->InstrumentID, instrument_id, sizeof(instrument_id));
  /// 交易日期
  char trading_day[27];
  utils::Gbk2Utf8(p_d->TradingDay, trading_day, sizeof(trading_day));
  /// 最后修改时间 TThostFtdcTimeType char[9]
  char update_time[27];
  utils::Gbk2Utf8(p_d->UpdateTime, update_time, sizeof(update_time));
  /// 最后修改毫秒 TThostFtdcMillisecType int
  int update_millisec = p_d->UpdateMillisec;
  /// 最新价 TThostFtdcPriceType double
  double last_price = Max2zero(p_d->LastPrice);
  /// 申买价一 TThostFtdcPriceType double
  double bid_price1 = Max2zero(p_d->BidPrice1);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume1 = p_d->BidVolume1;
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price1 = Max2zero(p_d->AskPrice1);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume1 = p_d->AskVolume1;
  /// 申买价一 TThostFtdcPriceType double
  double bid_price2 = Max2zero(p_d->BidPrice2);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume2 = p_d->BidVolume2;
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price2 = Max2zero(p_d->AskPrice2);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume2 = p_d->AskVolume2;
  /// 申买价一 TThostFtdcPriceType double
  double bid_price3 = Max2zero(p_d->BidPrice3);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume3 = p_d->BidVolume3;
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price3 = Max2zero(p_d->AskPrice3);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume3 = p_d->AskVolume3;
  /// 申买价一 TThostFtdcPriceType double
  double bid_price4 = Max2zero(p_d->BidPrice4);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume4 = p_d->BidVolume4;
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price4 = Max2zero(p_d->AskPrice4);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume4 = p_d->AskVolume4;
  /// 申买价一 TThostFtdcPriceType double
  double bid_price5 = Max2zero(p_d->BidPrice5);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume5 = p_d->BidVolume5;
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price5 = Max2zero(p_d->AskPrice5);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume5 = p_d->AskVolume5;
  /// 数量 TThostFtdcVolumeType int
  int volume = p_d->Volume;
  /// 成交金额 TThostFtdcMoneyType double
  double turnover = Max2zero(p_d->Turnover);
  /// 持仓量 TThostFtdcLargeVolumeType double
  double open_interest = Max2zero(p_d->OpenInterest);
  /// 涨停板价 TThostFtdcPriceType double
  double upper_limit_price = Max2zero(p_d->UpperLimitPrice);
  /// 跌停板价 TThostFtdcPriceType double
  double lower_limit_price = Max2zero(p_d->LowerLimitPrice);
  /// 今开盘 TThostFtdcPriceType double
  double open_price = Max2zero(p_d->OpenPrice);
  /// 上次结算价 TThostFtdcPriceType double
  double pre_settlement_price = Max2zero(p_d->PreSettlementPrice);
  /// 昨收盘 TThostFtdcPriceType double
  double pre_close_price = Max2zero(p_d->PreClosePrice);
  /// 昨持仓量 TThostFtdcLargeVolumeType double
  double pre_open_interest = Max2zero(p_d->PreOpenInterest);
  /// 结算价 SettlementPrice double
  double settlement_price = Max2zero(p_d->SettlementPrice);

  sprintf(dataflow_,
          "%s,%s,%s.%d,%.15g,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%d,%.15g,%.15g,%."
          "15g,%.15g,%.15g,%.15g,%.15g,%.5lf,%.15g",
          instrument_id, trading_day, update_time, update_millisec, last_price, bid_price1, bid_volume1, ask_price1, ask_volume1,
          bid_price2, bid_volume2, ask_price2, ask_volume2, bid_price3, bid_volume3, ask_price3, ask_volume3, bid_price4, bid_volume4,
          ask_price4, ask_volume4, bid_price5, bid_volume5, ask_price5, ask_volume5, volume, turnover, open_interest, upper_limit_price,
          lower_limit_price, open_price, pre_settlement_price, pre_close_price, pre_open_interest, settlement_price);
}

void LoadData::LoadDepthMarketDataToCsv(CThostFtdcDepthMarketDataField *p_d) {
  char csvpath[200];
  char instrument_id[93];
  uint8_t exist_flag = 1;

  utils::Gbk2Utf8(p_d->InstrumentID, instrument_id, sizeof(instrument_id));  // 合约代码
  if (instrument_id[2] == 'e' && instrument_id[3] == 'f' && instrument_id[4] == 'p') {
    return;
  }

  if (!IsValidTickData(p_d)) {
    return;
  }

  if (access(history_tick_folder_.c_str(), F_OK) == -1) {
    mkdir(history_tick_folder_.c_str(), S_IRWXU);
  }

  sprintf(csvpath, "%s/%s.csv", history_tick_folder_.c_str(),
          instrument_id);  // 合成存储路径

  FormDepthMarketData2Stringflow(p_d);

  if (access(csvpath, F_OK) == -1) {
    exist_flag = 0;
  }

  std::ofstream out(csvpath, std::ios::app);
  if (out.is_open()) {
    if (exist_flag == 0) {
      out << titleflow_ << "\r\r\n";
    }
    out << dataflow_ << "\r\r\n";
  } else {
    INFO_LOG("%s open fail.\n", csvpath);
    fflush(stdout);
  }

  out.close();  // 关闭文件
}

void LoadData::FormDepthMarketData2Stringflow(XTPMD *p_d) {
  /// 合约代码 TThostFtdcInstrumentIDType char[31]
  char instrument_id[16];
  utils::Gbk2Utf8(p_d->ticker, instrument_id, sizeof(instrument_id));
  string data_time = std::to_string(p_d->data_time);
  /// 交易日期
  string trading_day = data_time.substr(0, 8);
  /// 最后修改时间 TThostFtdcTimeType char[9]
  string update_time_hour = data_time.substr(8, 2);
  string update_time_min = data_time.substr(10, 2);
  string update_time_second = data_time.substr(12, 2);
  /// 最后修改毫秒 TThostFtdcMillisecType int
  string update_millisec = data_time.substr(14, 3);
  /// 最新价 TThostFtdcPriceType double
  double last_price = Max2zero(p_d->last_price);
  /// 申买价一 TThostFtdcPriceType double
  double bid_price1 = Max2zero(p_d->bid[0]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume1 = p_d->bid_qty[0];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price1 = Max2zero(p_d->ask[0]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume1 = p_d->ask_qty[0];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price2 = Max2zero(p_d->bid[1]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume2 = p_d->bid_qty[1];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price2 = Max2zero(p_d->ask[1]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume2 = p_d->ask_qty[1];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price3 = Max2zero(p_d->bid[2]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume3 = p_d->bid_qty[2];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price3 = Max2zero(p_d->ask[2]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume3 = p_d->ask_qty[2];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price4 = Max2zero(p_d->bid[3]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume4 = p_d->bid_qty[3];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price4 = Max2zero(p_d->ask[3]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume4 = p_d->ask_qty[3];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price5 = Max2zero(p_d->bid[4]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume5 = p_d->bid_qty[4];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price5 = Max2zero(p_d->ask[4]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume5 = p_d->ask_qty[4];
  /// 数量 TThostFtdcVolumeType int
  int64_t volume = p_d->trades_count;
  /// 成交金额 TThostFtdcMoneyType double
  double turnover = Max2zero(p_d->turnover);
  /// 持仓量 TThostFtdcLargeVolumeType double
  int64_t open_interest = Max2zero(p_d->total_long_positon);
  /// 涨停板价 TThostFtdcPriceType double
  double upper_limit_price = Max2zero(p_d->upper_limit_price);
  /// 跌停板价 TThostFtdcPriceType double
  double lower_limit_price = Max2zero(p_d->lower_limit_price);
  /// 今开盘 TThostFtdcPriceType double
  double open_price = Max2zero(p_d->open_price);
  /// 上次结算价 TThostFtdcPriceType double
  double pre_settlement_price = Max2zero(p_d->pre_settl_price);
  /// 昨收盘 TThostFtdcPriceType double
  double pre_close_price = Max2zero(p_d->pre_close_price);
  /// 昨持仓量 TThostFtdcLargeVolumeType double
  int64_t pre_open_interest = Max2zero(p_d->pre_total_long_positon);
  /// 结算价 SettlementPrice double
  double settlement_price = Max2zero(p_d->settl_price);

  sprintf(dataflow_,
          "%s,%s,%s:%s:%s.%s,%.15g,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%ld,%.15g,%ld,"
          "%.15g,%.15g,%.15g,%.15g,%.15g,%ld,%.15g",
          instrument_id, trading_day.c_str(), update_time_hour.c_str(), update_time_min.c_str(), update_time_second.c_str(),
          update_millisec.c_str(), last_price, bid_price1, bid_volume1, ask_price1, ask_volume1, bid_price2, bid_volume2, ask_price2,
          ask_volume2, bid_price3, bid_volume3, ask_price3, ask_volume3, bid_price4, bid_volume4, ask_price4, ask_volume4, bid_price5,
          bid_volume5, ask_price5, ask_volume5, volume, turnover, open_interest, upper_limit_price, lower_limit_price, open_price,
          pre_settlement_price, pre_close_price, pre_open_interest, settlement_price);
}

void LoadData::LoadDepthMarketDataToCsv(XTPMD *p_d) {
  char csvpath[200];
  char instrument_id[16];
  uint8_t exist_flag = 1;

  utils::Gbk2Utf8(p_d->ticker, instrument_id, sizeof(instrument_id));  // 合约代码

  if (!IsValidTickData(p_d)) {
    return;
  }

  if (access(history_tick_folder_.c_str(), F_OK) == -1) {
    mkdir(history_tick_folder_.c_str(), S_IRWXU);
  }

  sprintf(csvpath, "%s/%s.csv", history_tick_folder_.c_str(), instrument_id);  // 合成存储路径

  FormDepthMarketData2Stringflow(p_d);

  if (access(csvpath, F_OK) == -1) {
    exist_flag = 0;
  }

  std::ofstream out(csvpath, std::ios::app);
  if (out.is_open()) {
    if (exist_flag == 0) {
      out << titleflow_ << "\r\r\n";
    }
    out << dataflow_ << "\r\r\n";
  } else {
    INFO_LOG("%s open fail.\n", csvpath);
    fflush(stdout);
  }

  out.close();  // 关闭文件
}

void LoadData::FormDepthMarketData2Stringflow(MdsMktDataSnapshotT *p_d) {
  /// 合约代码 TThostFtdcInstrumentIDType char[31]
  char instrument_id[16];
  sprintf(instrument_id, "%06d", p_d->head.instrId);
  /// 交易日期
  string trading_day = std::to_string(p_d->head.tradeDate);
  /// 最后修改时间 TThostFtdcTimeType char[9]
  int update_time_hour = p_d->head.updateTime / 10000000;
  int update_time_min = (p_d->head.updateTime / 100000) % 100;
  int update_time_second = (p_d->head.updateTime / 1000) % 100;
  /// 最后修改毫秒 TThostFtdcMillisecType int
  int update_millisec = p_d->head.updateTime % 1000;
  /// 最新价 TThostFtdcPriceType double
  double last_price = p_d->l2Stock.TradePx * 0.0001;
  /// 申买价一 TThostFtdcPriceType double
  double bid_price1 = p_d->l2Stock.BidLevels[0].Price * 0.0001;
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume1 = p_d->l2Stock.BidLevels[0].NumberOfOrders;
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price1 = p_d->l2Stock.OfferLevels[0].Price * 0.0001;
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume1 = p_d->l2Stock.OfferLevels[0].NumberOfOrders;
  /// 申买价一 TThostFtdcPriceType double
  double bid_price2 = p_d->l2Stock.BidLevels[1].Price * 0.0001;
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume2 = p_d->l2Stock.BidLevels[1].NumberOfOrders;
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price2 = p_d->l2Stock.OfferLevels[1].Price * 0.0001;
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume2 = p_d->l2Stock.OfferLevels[1].NumberOfOrders;
  /// 申买价一 TThostFtdcPriceType double
  double bid_price3 = p_d->l2Stock.BidLevels[2].Price * 0.0001;
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume3 = p_d->l2Stock.BidLevels[2].NumberOfOrders;
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price3 = p_d->l2Stock.OfferLevels[2].Price * 0.0001;
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume3 = p_d->l2Stock.OfferLevels[2].NumberOfOrders;
  /// 申买价一 TThostFtdcPriceType double
  double bid_price4 = p_d->l2Stock.BidLevels[3].Price * 0.0001;
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume4 = p_d->l2Stock.BidLevels[3].NumberOfOrders;
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price4 = p_d->l2Stock.OfferLevels[3].Price * 0.0001;
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume4 = p_d->l2Stock.OfferLevels[3].NumberOfOrders;
  /// 申买价一 TThostFtdcPriceType double
  double bid_price5 = p_d->l2Stock.BidLevels[4].Price * 0.0001;
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume5 = p_d->l2Stock.BidLevels[4].NumberOfOrders;
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price5 = p_d->l2Stock.OfferLevels[4].Price * 0.0001;
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume5 = p_d->l2Stock.OfferLevels[4].NumberOfOrders;
  /// 数量 TThostFtdcVolumeType int
  int64_t volume = p_d->l2Stock.TotalVolumeTraded;
  /// 成交金额 TThostFtdcMoneyType double
  double turnover = p_d->l2Stock.TotalValueTraded * 0.0001;
  /// 持仓量 TThostFtdcLargeVolumeType double
  int64_t open_interest = 0;
  /// 涨停板价 TThostFtdcPriceType double
  double upper_limit_price = 0;
  /// 跌停板价 TThostFtdcPriceType double
  double lower_limit_price = 0;
  /// 今开盘 TThostFtdcPriceType double
  double open_price = p_d->l2Stock.OpenPx * 0.0001;
  /// 上次结算价 TThostFtdcPriceType double
  double pre_settlement_price = 0;
  /// 昨收盘 TThostFtdcPriceType double
  double pre_close_price = p_d->l2Stock.PrevClosePx * 0.0001;
  /// 昨持仓量 TThostFtdcLargeVolumeType double
  int64_t pre_open_interest = 0;
  /// 结算价 SettlementPrice double
  double settlement_price = 0;

  sprintf(dataflow_,
          "%s,%s,%02d:%02d:%02d.%03d,%.15g,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%ld,%."
          "15g,%ld,%.15g,%.15g,%.15g,%.15g,%.15g,%ld,%.15g",
          instrument_id, trading_day.c_str(), update_time_hour, update_time_min, update_time_second, update_millisec, last_price,
          bid_price1, bid_volume1, ask_price1, ask_volume1, bid_price2, bid_volume2, ask_price2, ask_volume2, bid_price3, bid_volume3,
          ask_price3, ask_volume3, bid_price4, bid_volume4, ask_price4, ask_volume4, bid_price5, bid_volume5, ask_price5, ask_volume5,
          volume, turnover, open_interest, upper_limit_price, lower_limit_price, open_price, pre_settlement_price, pre_close_price,
          pre_open_interest, settlement_price);
}

void LoadData::LoadDepthMarketDataToCsv(MdsMktDataSnapshotT *p_d) {
  char csvpath[200];
  char instrument_id[16];
  uint8_t exist_flag = 1;

  sprintf(instrument_id, "%06d", p_d->head.instrId);

  if (!IsValidTickData(p_d)) {
    return;
  }

  if (access(history_tick_folder_.c_str(), F_OK) == -1) {
    mkdir(history_tick_folder_.c_str(), S_IRWXU);
  }

  sprintf(csvpath, "%s/%s.csv", history_tick_folder_.c_str(),
          instrument_id);  // 合成存储路径

  FormDepthMarketData2Stringflow(p_d);

  if (access(csvpath, F_OK) == -1) {
    exist_flag = 0;
  }

  std::ofstream out(csvpath, std::ios::app);
  if (out.is_open()) {
    if (exist_flag == 0) {
      out << titleflow_ << "\r\r\n";
    }
    out << dataflow_ << "\r\r\n";
  } else {
    INFO_LOG("%s open fail.\n", csvpath);
    fflush(stdout);
  }

  out.close();  // 关闭文件
}

void LoadData::FormDepthMarketData2Stringflow(GtpMarketDataStruct *p_d) {
  string data_time = std::string(p_d->date_time);
  /// 交易日期
  string trading_year = data_time.substr(0, 4);
  string trading_month = data_time.substr(5, 2);
  string trading_date = data_time.substr(8, 2);
  /// 最后修改时间 TThostFtdcTimeType char[9]
  string update_time = data_time.substr(11);
  /// 最新价 TThostFtdcPriceType double
  double last_price = Max2zero(p_d->last_price);
  /// 申买价一 TThostFtdcPriceType double
  double bid_price1 = Max2zero(p_d->bid_price[0]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume1 = p_d->bid_volume[0];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price1 = Max2zero(p_d->ask_price[0]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume1 = p_d->ask_volume[0];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price2 = Max2zero(p_d->bid_price[1]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume2 = p_d->bid_volume[1];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price2 = Max2zero(p_d->ask_price[1]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume2 = p_d->ask_volume[1];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price3 = Max2zero(p_d->bid_price[2]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume3 = p_d->bid_volume[2];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price3 = Max2zero(p_d->ask_price[2]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume3 = p_d->ask_volume[2];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price4 = Max2zero(p_d->bid_price[3]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume4 = p_d->bid_volume[3];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price4 = Max2zero(p_d->ask_price[3]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume4 = p_d->ask_volume[3];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price5 = Max2zero(p_d->bid_price[4]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume5 = p_d->bid_volume[4];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price5 = Max2zero(p_d->ask_price[4]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume5 = p_d->ask_volume[4];
  /// 数量 TThostFtdcVolumeType int
  int64_t volume = p_d->volume;
  /// 持仓量 TThostFtdcLargeVolumeType double
  int64_t open_interest = p_d->positon;

  sprintf(dataflow_,
          "%s,%s%s%s,%s,%.15g,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%ld,,%ld,,,,,,,",
          p_d->instrument_id, trading_year.c_str(), trading_month.c_str(), trading_date.c_str(), update_time.c_str(), last_price,
          bid_price1, bid_volume1, ask_price1, ask_volume1, bid_price2, bid_volume2, ask_price2, ask_volume2, bid_price3, bid_volume3,
          ask_price3, ask_volume3, bid_price4, bid_volume4, ask_price4, ask_volume4, bid_price5, bid_volume5, ask_price5, ask_volume5,
          volume, open_interest);
}

void LoadData::LoadDepthMarketDataToCsv(GtpMarketDataStruct *p_d) {
  char csvpath[200];
  uint8_t exist_flag = 1;

  if (!IsValidTickData(p_d)) {
    return;
  }

  if (access(history_tick_folder_.c_str(), F_OK) == -1) {
    mkdir(history_tick_folder_.c_str(), S_IRWXU);
  }

  sprintf(csvpath, "%s/%s.csv", history_tick_folder_.c_str(),
          p_d->instrument_id);  // 合成存储路径

  FormDepthMarketData2Stringflow(p_d);

  if (access(csvpath, F_OK) == -1) {
    exist_flag = 0;
  }

  std::ofstream out(csvpath, std::ios::app);
  if (out.is_open()) {
    if (exist_flag == 0) {
      out << titleflow_ << "\r\r\n";
    }
    out << dataflow_ << "\r\r\n";
  } else {
    INFO_LOG("%s open fail.\n", csvpath);
    fflush(stdout);
  }

  out.close();  // 关闭文件
}

void LoadData::FormDepthMarketData2Stringflow(MtpMarketDataStruct *p_d) {
  string data_time = std::string(p_d->date_time);
  /// 交易日期
  string trading_year = data_time.substr(0, 4);
  string trading_month = data_time.substr(5, 2);
  string trading_date = data_time.substr(8, 2);
  /// 最后修改时间 TThostFtdcTimeType char[9]
  string update_time = data_time.substr(11);
  /// 最新价 TThostFtdcPriceType double
  double last_price = Max2zero(p_d->last_price);
  /// 申买价一 TThostFtdcPriceType double
  double bid_price1 = Max2zero(p_d->bid_price[0]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume1 = p_d->bid_volume[0];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price1 = Max2zero(p_d->ask_price[0]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume1 = p_d->ask_volume[0];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price2 = Max2zero(p_d->bid_price[1]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume2 = p_d->bid_volume[1];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price2 = Max2zero(p_d->ask_price[1]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume2 = p_d->ask_volume[1];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price3 = Max2zero(p_d->bid_price[2]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume3 = p_d->bid_volume[2];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price3 = Max2zero(p_d->ask_price[2]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume3 = p_d->ask_volume[2];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price4 = Max2zero(p_d->bid_price[3]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume4 = p_d->bid_volume[3];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price4 = Max2zero(p_d->ask_price[3]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume4 = p_d->ask_volume[3];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price5 = Max2zero(p_d->bid_price[4]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume5 = p_d->bid_volume[4];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price5 = Max2zero(p_d->ask_price[4]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume5 = p_d->ask_volume[4];
  /// 数量 TThostFtdcVolumeType int
  int64_t volume = p_d->volume;
  /// 持仓量 TThostFtdcLargeVolumeType double
  int64_t open_interest = p_d->positon;

  sprintf(dataflow_,
          "%s,%s%s%s,%s,%.15g,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%ld,,%ld,,,,,,,",
          p_d->instrument_id, trading_year.c_str(), trading_month.c_str(), trading_date.c_str(), update_time.c_str(), last_price,
          bid_price1, bid_volume1, ask_price1, ask_volume1, bid_price2, bid_volume2, ask_price2, ask_volume2, bid_price3, bid_volume3,
          ask_price3, ask_volume3, bid_price4, bid_volume4, ask_price4, ask_volume4, bid_price5, bid_volume5, ask_price5, ask_volume5,
          volume, open_interest);
}

void LoadData::LoadDepthMarketDataToCsv(MtpMarketDataStruct *p_d) {
  char csvpath[200];
  uint8_t exist_flag = 1;

  if (!IsValidTickData(p_d)) {
    return;
  }

  if (access(history_tick_folder_.c_str(), F_OK) == -1) {
    mkdir(history_tick_folder_.c_str(), S_IRWXU);
  }

  sprintf(csvpath, "%s/%s.csv", history_tick_folder_.c_str(),
          p_d->instrument_id);  // 合成存储路径

  FormDepthMarketData2Stringflow(p_d);

  if (access(csvpath, F_OK) == -1) {
    exist_flag = 0;
  }

  std::ofstream out(csvpath, std::ios::app);
  if (out.is_open()) {
    if (exist_flag == 0) {
      out << titleflow_ << "\r\r\n";
    }
    out << dataflow_ << "\r\r\n";
  } else {
    INFO_LOG("%s open fail.\n", csvpath);
    fflush(stdout);
  }

  out.close();  // 关闭文件
}

void LoadData::FormDepthMarketData2Stringflow(YtpMarketDataStruct *p_d) {
  string data_time = std::string(p_d->date_time);
  /// 交易日期
  string trading_year = data_time.substr(0, 4);
  string trading_month = data_time.substr(5, 2);
  string trading_date = data_time.substr(8, 2);
  /// 最后修改时间 TThostFtdcTimeType char[9]
  string update_time = data_time.substr(11);
  /// 最新价 TThostFtdcPriceType double
  double last_price = Max2zero(p_d->last_price);
  /// 申买价一 TThostFtdcPriceType double
  double bid_price1 = Max2zero(p_d->bid_price[0]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume1 = p_d->bid_volume[0];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price1 = Max2zero(p_d->ask_price[0]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume1 = p_d->ask_volume[0];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price2 = Max2zero(p_d->bid_price[1]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume2 = p_d->bid_volume[1];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price2 = Max2zero(p_d->ask_price[1]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume2 = p_d->ask_volume[1];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price3 = Max2zero(p_d->bid_price[2]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume3 = p_d->bid_volume[2];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price3 = Max2zero(p_d->ask_price[2]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume3 = p_d->ask_volume[2];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price4 = Max2zero(p_d->bid_price[3]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume4 = p_d->bid_volume[3];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price4 = Max2zero(p_d->ask_price[3]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume4 = p_d->ask_volume[3];
  /// 申买价一 TThostFtdcPriceType double
  double bid_price5 = Max2zero(p_d->bid_price[4]);
  /// 申买量一 TThostFtdcVolumeType int
  int bid_volume5 = p_d->bid_volume[4];
  /// 申卖价一 TThostFtdcPriceType double
  double ask_price5 = Max2zero(p_d->ask_price[4]);
  /// 申卖量一 TThostFtdcVolumeType int
  int ask_volume5 = p_d->ask_volume[4];
  /// 数量 TThostFtdcVolumeType int
  int64_t volume = p_d->volume;
  /// 持仓量 TThostFtdcLargeVolumeType double
  int64_t open_interest = p_d->positon;

  sprintf(dataflow_,
          "%s,%s%s%s,%s,%.15g,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%.15g,%d,%ld,,%ld,,,,,,,",
          p_d->instrument_id, trading_year.c_str(), trading_month.c_str(), trading_date.c_str(), update_time.c_str(), last_price,
          bid_price1, bid_volume1, ask_price1, ask_volume1, bid_price2, bid_volume2, ask_price2, ask_volume2, bid_price3, bid_volume3,
          ask_price3, ask_volume3, bid_price4, bid_volume4, ask_price4, ask_volume4, bid_price5, bid_volume5, ask_price5, ask_volume5,
          volume, open_interest);
}

void LoadData::LoadDepthMarketDataToCsv(YtpMarketDataStruct *p_d) {
  char csvpath[200];
  uint8_t exist_flag = 1;

  if (!IsValidTickData(p_d)) {
    return;
  }

  if (access(history_tick_folder_.c_str(), F_OK) == -1) {
    mkdir(history_tick_folder_.c_str(), S_IRWXU);
  }

  sprintf(csvpath, "%s/%s.csv", history_tick_folder_.c_str(),
          p_d->instrument_id);  // 合成存储路径

  FormDepthMarketData2Stringflow(p_d);

  if (access(csvpath, F_OK) == -1) {
    exist_flag = 0;
  }

  std::ofstream out(csvpath, std::ios::app);
  if (out.is_open()) {
    if (exist_flag == 0) {
      out << titleflow_ << "\r\r\n";
    }
    out << dataflow_ << "\r\r\n";
  } else {
    INFO_LOG("%s open fail.\n", csvpath);
    fflush(stdout);
  }

  out.close();  // 关闭文件
}

bool LoadData::ClassifyContractFiles(void) {
  struct dirent *filename;  // return value for readdir()
  DIR *dir;                 // return value for opendir()
  dir = opendir(history_tick_folder_.c_str());
  if (NULL == dir) {
    ERROR_LOG("can not open dir ");
    return false;
  }

  while ((filename = readdir(dir)) != NULL) {
    if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0) continue;

    // 在map中查找合约
    string contract_file = filename->d_name;
    int pos = contract_file.find_first_of('.');
    string file_name(contract_file.substr(0, pos));
    string file_format(contract_file.substr(pos + 1));

    if ((pos == -1) || (strcmp(file_format.c_str(), "csv") != 0)) {
      continue;
    }

    auto &market_ser = MarketService::GetInstance();
    string exch = market_ser.ROLE(InstrumentInfo).GetExchange(file_name);
    if (exch != "") {
      MoveContractToFolder(contract_file, exch);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } else {
      utils::DeleteFile(history_tick_folder_ + contract_file);
      WARNING_LOG("not found file name: %s", file_name.c_str());
    }
  }

  closedir(dir);
  return true;
}

bool LoadData::MoveContractToFolder(const string &contract_name, const string &exchange_name) {
  char csvpath[200];
  char folderpath[180];
  char command[400];

  sprintf(csvpath, "%s/%s", history_tick_folder_.c_str(),
          contract_name.c_str());  // 合成存储路径
  sprintf(folderpath, "%s/%s/", history_tick_folder_.c_str(),
          exchange_name.c_str());  // 合成存储文件夹路径

  if (access(folderpath, F_OK) == -1) {
    if (mkdir(folderpath, S_IRWXU) == 0) {
      INFO_LOG("create folder %s ok.", folderpath);
      fflush(stdout);
    } else {
      ERROR_LOG("create folder %s fail.", folderpath);
      fflush(stdout);
      return false;
    }
  }

  sprintf(command, "mv %s %s", csvpath, folderpath);
  if (system(command) == -1) {
    ERROR_LOG("%s", command);
  }
  return true;
}
