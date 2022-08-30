#include "market/infra/sender/xtpSender.h"
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/self/fileUtil.h"
#include "common/self/semaphorePart.h"
#include "common/self/utils.h"
#include "market/infra/recer/xtpRecer.h"

XTP::API::QuoteApi *XtpSender::quoteApi;
XtpQuoteSpi *XtpSender::quoteSpi;

XtpSender::XtpSender(void) { ; }

bool XtpSender::init(void) {
  bool out = true;
  if (isInit == false) {
    auto &jsonCfg = utils::JsonConfig::getInstance();
    auto users = jsonCfg.getConfig("market", "User");
    for (auto &user : users) {
      conPath = jsonCfg.getConfig("market", "ConPath").get<std::string>() + "/" + (std::string)user;
      utils::creatFolder(conPath);

      uint8_t client_id = jsonCfg.getConfig("common", "ClientId").get<std::uint8_t>();
      quoteApi = XTP::API::QuoteApi::CreateQuoteApi(client_id, conPath.c_str(), XTP_LOG_LEVEL_DEBUG);
      INFO_LOG("xtp version: %s.", quoteApi->GetApiVersion());

      quoteSpi = new XtpQuoteSpi();
      quoteApi->RegisterSpi(quoteSpi);

      //设定行情服务器超时时间，单位为秒
      quoteApi->SetHeartBeatInterval(120);
      //设定行情本地缓存大小，单位为MB
      quoteApi->SetUDPBufferSize(1);

      INFO_LOG("quoteApi init ok.");
      std::this_thread::sleep_for(1000ms);
      break;
    }
    isInit = true;
  }
  return out;
}

bool XtpSender::ReqUserLogin(void) {
  INFO_LOG("login time, is going to login.");
  bool ret = true;
  init();
  auto &jsonCfg = utils::JsonConfig::getInstance();
  auto users = jsonCfg.getConfig("market", "User");
  for (auto &user : users) {
    std::string frontaddr = jsonCfg.getDeepConfig("users", (std::string)user, "FrontMdAddr").get<std::string>();
    const std::string userID = jsonCfg.getDeepConfig("users", (std::string)user, "UserID").get<std::string>();
    const std::string password = jsonCfg.getDeepConfig("users", (std::string)user, "Password").get<std::string>();

    auto protocol_ip_port = utils::splitString(frontaddr, ":");
    std::string ip = utils::splitString(protocol_ip_port[1], "//")[1];
    int port = utils::stringToInt(protocol_ip_port[2]);
    XTP_PROTOCOL_TYPE protoc = XTP_PROTOCOL_TCP;
    if (protocol_ip_port[0] == "tcp") {
      protoc = XTP_PROTOCOL_TCP;
    } else if (protocol_ip_port[0] == "udp") {
      protoc = XTP_PROTOCOL_UDP;
    }

    int loginResult_quote = quoteApi->Login(ip.c_str(), port, userID.c_str(), password.c_str(), protoc);
    if (loginResult_quote == -1) {
      XTPRI *error_info = quoteApi->GetApiLastError();
      ERROR_LOG("Login to server error: %d : %s", error_info->error_id, error_info->error_msg);
      release();
      ret = false;
    } else {
      auto &globalSem = GlobalSem::getInstance();
      if (globalSem.waitSemBySemName(GlobalSem::loginLogout, 3) != 0) {
        quoteSpi->OnRspUserLogin();
      }
    }

    break;
  }
  return ret;
}

bool XtpSender::ReqUserLogout() {
  INFO_LOG("logout time, is going to logout.");

  int result = quoteApi->Logout();
  INFO_LOG("ReqUserLogout send result is [%d]", result);

  auto &globalSem = GlobalSem::getInstance();
  if (globalSem.waitSemBySemName(GlobalSem::loginLogout, 3) != 0) {
    quoteSpi->OnRspUserLogout();
  }

  release();
}

bool XtpSender::release() {
  INFO_LOG("Is going to release quoteApi.");

  quoteApi->Release();
  quoteApi = nullptr;

  // 释放UserSpi实例
  if (quoteSpi) {
    delete quoteSpi;
    quoteSpi = NULL;
  }
  isInit = false;
}

bool XtpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec) {
  int result = true;
  if (nameVec.size() > 500) {
    WARNING_LOG("too much instruments to unSubscription.");
    return result;
  }

  unsigned int sh_count = 0, sz_sount = 0;
  char **ppInstrumentID_SH = new char *[nameVec.size()];
  char **ppInstrumentID_SZ = new char *[nameVec.size()];

  for (auto &item : nameVec) {
    if (item.exch == "SHSE") {
      ppInstrumentID_SH[sh_count] = const_cast<char *>(item.ins.c_str());
      sh_count++;
    } else if (item.exch == "SZSE") {
      ppInstrumentID_SZ[sz_sount] = const_cast<char *>(item.ins.c_str());
      sz_sount++;
    } else {
      ERROR_LOG("not have this exchange.");
    }
  }

  if (sh_count > 0) {
    result = quoteApi->SubscribeMarketData(ppInstrumentID_SH, sh_count, XTP_EXCHANGE_SH);
    if (result != 0) {
      ERROR_LOG("subscribeMarketData fail, error code[%d]", result);
    }
  }

  if (sz_sount > 0) {
    result = quoteApi->SubscribeMarketData(ppInstrumentID_SZ, sz_sount, XTP_EXCHANGE_SZ);
    if (result != 0) {
      ERROR_LOG("subscribeMarketData fail, error code[%d]", result);
    }
  }

  delete[] ppInstrumentID_SH;
  delete[] ppInstrumentID_SZ;
}

bool XtpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec) {
  int result = true;
  if (nameVec.size() > 500) {
    WARNING_LOG("too much instruments to unSubscription.");
    return result;
  }

  unsigned int sh_count = 0, sz_sount = 0;
  char **ppInstrumentID_SH = new char *[nameVec.size()];
  char **ppInstrumentID_SZ = new char *[nameVec.size()];

  for (auto &item : nameVec) {
    if (item.exch == "SHSE") {
      ppInstrumentID_SH[sh_count] = const_cast<char *>(item.ins.c_str());
      sh_count++;
    } else if (item.exch == "SZSE") {
      ppInstrumentID_SZ[sz_sount] = const_cast<char *>(item.ins.c_str());
      sz_sount++;
    } else {
      ERROR_LOG("not have this exchange.");
    }
  }

  if (sh_count > 0) {
    result = quoteApi->UnSubscribeMarketData(ppInstrumentID_SH, sh_count, XTP_EXCHANGE_SH);
    if (result != 0) {
      ERROR_LOG("subscribeMarketData fail, error code[%d]", result);
    }
  }

  if (sz_sount > 0) {
    result = quoteApi->UnSubscribeMarketData(ppInstrumentID_SZ, sz_sount, XTP_EXCHANGE_SZ);
    if (result != 0) {
      ERROR_LOG("subscribeMarketData fail, error code[%d]", result);
    }
  }

  delete[] ppInstrumentID_SH;
  delete[] ppInstrumentID_SZ;
}

bool XtpSender::ReqInstrumentInfo(const utils::InstrumtntID &ins) {
  if (ins.exch == "SHSE") {
    if (ins.ins == "*") {
      int result = quoteApi->QueryAllTickers(XTP_EXCHANGE_SH);
      if (result != 0) {
        ERROR_LOG("request full shse market instruments, result[%d]", result);
      }
    }
  } else if (ins.exch == "SZSE") {
    if (ins.ins == "*") {
      int result = quoteApi->QueryAllTickers(XTP_EXCHANGE_SZ);
      if (result != 0) {
        ERROR_LOG("request full szse market instruments, result[%d]", result);
      }
    }
  }
}

bool XtpSender::LossConnection() { return (quoteSpi != nullptr && quoteSpi->frontDisconnected == true); }