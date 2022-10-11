#include "market/infra/sender/xtp_sender.h"
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/infra/recer/xtp_recer.h"

XTP::API::QuoteApi *XtpSender::quote_api;
XtpQuoteSpi *XtpSender::quote_spi;

XtpSender::XtpSender(void) { ; }

bool XtpSender::Init(void) {
  bool out = true;
  if (is_init_ == false) {
    auto &json_cfg = utils::JsonConfig::GetInstance();
    auto users = json_cfg.GetConfig("market", "User");
    for (auto &user : users) {
      con_path_ = json_cfg.GetConfig("market", "ConPath").get<std::string>() + "/" + (std::string)user;
      utils::CreatFolder(con_path_);

      uint8_t client_id = json_cfg.GetConfig("common", "ClientId").get<std::uint8_t>();
      quote_api = XTP::API::QuoteApi::CreateQuoteApi(client_id, con_path_.c_str(), XTP_LOG_LEVEL_DEBUG);
      INFO_LOG("xtp version: %s.", quote_api->GetApiVersion());

      quote_spi = new XtpQuoteSpi();
      quote_api->RegisterSpi(quote_spi);

      //设定行情服务器超时时间，单位为秒
      quote_api->SetHeartBeatInterval(120);
      //设定行情本地缓存大小，单位为MB
      quote_api->SetUDPBufferSize(1);

      INFO_LOG("quote_api init ok.");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      break;
    }
    is_init_ = true;
  }
  return out;
}

bool XtpSender::ReqUserLogin(void) {
  INFO_LOG("login time, is going to login.");
  bool ret = true;
  Init();
  auto &json_cfg = utils::JsonConfig::GetInstance();
  auto users = json_cfg.GetConfig("market", "User");
  for (auto &user : users) {
    std::string frontaddr = json_cfg.GetDeepConfig("users", (std::string)user, "FrontMdAddr").get<std::string>();
    std::string user_id = json_cfg.GetDeepConfig("users", (std::string)user, "UserID").get<std::string>();
    std::string password = json_cfg.GetDeepConfig("users", (std::string)user, "Password").get<std::string>();

    auto protocol_ip_port = utils::SplitString(frontaddr, ":");
    std::string ip_str = utils::SplitString(protocol_ip_port[1], "//")[1];
    int port = stoi(protocol_ip_port[2]);
    XTP_PROTOCOL_TYPE protoc = XTP_PROTOCOL_TCP;
    if (protocol_ip_port[0] == "tcp") {
      protoc = XTP_PROTOCOL_TCP;
    } else if (protocol_ip_port[0] == "udp") {
      protoc = XTP_PROTOCOL_UDP;
    }

    int login_result_quote = quote_api->Login(ip_str.c_str(), port, user_id.c_str(), password.c_str(), protoc);
    if (login_result_quote == -1) {
      XTPRI *error_info = quote_api->GetApiLastError();
      ERROR_LOG("Login to server error: %d : %s", error_info->error_id, error_info->error_msg);
      Release();
      ret = false;
    } else {
      UpdateInstrumentInfoFromMarket();
      auto &global_sem = GlobalSem::GetInstance();
      if (global_sem.WaitSemBySemName(GlobalSem::kLoginLogout, 3) != 0) {
        quote_spi->OnRspUserLogin();
      }
    }

    break;
  }
  return ret;
}

bool XtpSender::ReqUserLogout() {
  INFO_LOG("logout time, is going to logout.");

  if (quote_api != nullptr) {
    int result = quote_api->Logout();
    INFO_LOG("ReqUserLogout send result is [%d]", result);

    auto &global_sem = GlobalSem::GetInstance();
    if (global_sem.WaitSemBySemName(GlobalSem::kLoginLogout, 3) != 0) {
      quote_spi->OnRspUserLogout();
    }

    Release();
  }

  return true;
}

bool XtpSender::Release() {
  INFO_LOG("Is going to release quote_api.");

  if (quote_api != nullptr) {
    quote_api->Release();
    quote_api = nullptr;
  }

  // 释放UserSpi实例
  if (quote_spi != nullptr) {
    delete quote_spi;
    quote_spi = nullptr;
  }
  is_init_ = false;

  return true;
}

bool XtpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
  int result = true;
  if (name_vec.size() > 500) {
    WARNING_LOG("too much instruments to unSubscription.");
    return result;
  }

  unsigned int sh_count = 0, sz_sount = 0;
  char **pp_instrument_id_sh = new char *[name_vec.size()];
  char **pp_instrument_id_sz = new char *[name_vec.size()];

  for (auto &item : name_vec) {
    if (item.exch == "SHSE") {
      pp_instrument_id_sh[sh_count] = const_cast<char *>(item.ins.c_str());
      sh_count++;
    } else if (item.exch == "SZSE") {
      pp_instrument_id_sz[sz_sount] = const_cast<char *>(item.ins.c_str());
      sz_sount++;
    } else {
      ERROR_LOG("not have this exchange.");
    }
  }

  if (sh_count > 0) {
    result = quote_api->SubscribeMarketData(pp_instrument_id_sh, sh_count, XTP_EXCHANGE_SH);
    if (result == 0) {
      INFO_LOG("Subscription request ......Send a success, total number: %d", sh_count);
    } else {
      ERROR_LOG("subscribeMarketData fail, error code[%d]", result);
    }
  }

  if (sz_sount > 0) {
    result = quote_api->SubscribeMarketData(pp_instrument_id_sz, sz_sount, XTP_EXCHANGE_SZ);
    if (result == 0) {
      INFO_LOG("Subscription request ......Send a success, total number: %d", sz_sount);
    } else {
      ERROR_LOG("subscribeMarketData fail, error code[%d]", result);
    }
  }

  delete[] pp_instrument_id_sh;
  delete[] pp_instrument_id_sz;

  return true;
}

bool XtpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
  int result = true;
  if (name_vec.size() > 500) {
    WARNING_LOG("too much instruments to unSubscription.");
    return result;
  }

  unsigned int sh_count = 0, sz_sount = 0;
  char **pp_instrument_id_sh = new char *[name_vec.size()];
  char **pp_instrument_id_sz = new char *[name_vec.size()];

  for (auto &item : name_vec) {
    if (item.exch == "SHSE") {
      pp_instrument_id_sh[sh_count] = const_cast<char *>(item.ins.c_str());
      sh_count++;
    } else if (item.exch == "SZSE") {
      pp_instrument_id_sz[sz_sount] = const_cast<char *>(item.ins.c_str());
      sz_sount++;
    } else {
      ERROR_LOG("not have this exchange.");
    }
  }

  if (sh_count > 0) {
    result = quote_api->UnSubscribeMarketData(pp_instrument_id_sh, sh_count, XTP_EXCHANGE_SH);
    if (result != 0) {
      ERROR_LOG("subscribeMarketData fail, error code[%d]", result);
    }
  }

  if (sz_sount > 0) {
    result = quote_api->UnSubscribeMarketData(pp_instrument_id_sz, sz_sount, XTP_EXCHANGE_SZ);
    if (result != 0) {
      ERROR_LOG("subscribeMarketData fail, error code[%d]", result);
    }
  }

  delete[] pp_instrument_id_sh;
  delete[] pp_instrument_id_sz;

  return true;
}

void XtpSender::UpdateInstrumentInfoFromMarket() {
  auto &global_sem = GlobalSem::GetInstance();

  int result = quote_api->QueryAllTickers(XTP_EXCHANGE_SH);
  if (result != 0) {
    ERROR_LOG("request full shse market instruments, result[%d]", result);
  }
  global_sem.WaitSemBySemName(GlobalSem::kUpdateInstrumentInfo);

  result = quote_api->QueryAllTickers(XTP_EXCHANGE_SZ);
  if (result != 0) {
    ERROR_LOG("request full szse market instruments, result[%d]", result);
  }
  global_sem.WaitSemBySemName(GlobalSem::kUpdateInstrumentInfo);

  INFO_LOG("UpdateInstrumentInfoFromMarket ok");
}

bool XtpSender::ReqInstrumentInfo(const utils::InstrumtntID &ins, const int request_id) {
  XTPQSI ticker_info;
  strcpy(ticker_info.ticker, ins.ins.c_str());
  if (ins.exch == "SZSE") {
    ticker_info.exchange_id = XTP_EXCHANGE_SZ;
  } else if (ins.exch == "SHSE") {
    ticker_info.exchange_id = XTP_EXCHANGE_SH;
  } else {
    ticker_info.exchange_id = XTP_EXCHANGE_UNKNOWN;
  }

  quote_spi->OnRspInstrumentInfo(&ticker_info, request_id);
  return true;
}

bool XtpSender::LossConnection() { return (quote_spi != nullptr && quote_spi->front_disconnected == true); }