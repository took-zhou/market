/*
 * ctpSender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/ctpSender.h"
#include <string>
#include "common/extern/log/log.h"
#include "common/self/fileUtil.h"
#include "common/self/semaphorePart.h"
#include "common/self/utils.h"
#include "market/infra/recer/ctpRecer.h"

CThostFtdcMdApi *CtpSender::marketApi;
CtpMarketSpi *CtpSender::marketSpi;

CtpSender::CtpSender(void) { ; }

bool CtpSender::init(void) {
  bool out = true;
  if (isInit == false) {
    auto &jsonCfg = utils::JsonConfig::getInstance();
    auto users = jsonCfg.getConfig("market", "User");
    for (auto &user : users) {
      conPath = jsonCfg.getConfig("market", "ConPath").get<std::string>() + "/" + (std::string)user + "/";
      utils::creatFolder(conPath);
      marketApi = CThostFtdcMdApi::CreateFtdcMdApi(conPath.c_str(), true, true);
      INFO_LOG("ctp version: %s.", marketApi->GetApiVersion());

      marketSpi = new CtpMarketSpi();
      marketApi->RegisterSpi(marketSpi);

      std::string frontaddr = jsonCfg.getDeepConfig("users", (std::string)user, "FrontMdAddr").get<std::string>();

      marketApi->RegisterFront(const_cast<char *>(frontaddr.c_str()));
      marketApi->Init();

      auto &globalSem = GlobalSem::getInstance();
      if (globalSem.waitSemBySemName(GlobalSem::loginLogout, 3)) {
        out = false;
        ERROR_LOG("market init fail.");
      } else {
        out = true;
        INFO_LOG("market init ok.");
      }
      break;
    }
    isInit = true;
  }

  return out;
}

bool CtpSender::release() {
  INFO_LOG("Is going to release marketApi.");
  marketApi->Release();

  // 释放UserSpi实例
  if (marketSpi) {
    delete marketSpi;
    marketSpi = NULL;
  }

  isInit = false;
}

bool CtpSender::ReqUserLogin() {
  bool ret = true;
  if (init() == false) {
    ret = false;
    release();
  } else {
    CThostFtdcReqUserLoginField reqUserLogin = {0};
    auto &jsonCfg = utils::JsonConfig::getInstance();

    auto users = jsonCfg.getConfig("market", "User");
    for (auto &user : users) {
      const std::string userID = jsonCfg.getDeepConfig("users", (std::string)user, "UserID");
      const std::string brokerID = jsonCfg.getDeepConfig("users", (std::string)user, "BrokerID");
      const std::string passWord = jsonCfg.getDeepConfig("users", (std::string)user, "Password");
      strcpy(reqUserLogin.BrokerID, brokerID.c_str());
      strcpy(reqUserLogin.UserID, userID.c_str());
      strcpy(reqUserLogin.Password, passWord.c_str());

      int result = marketApi->ReqUserLogin(&reqUserLogin, nRequestID++);
      if (result != 0) {
        INFO_LOG("ReqUserLogin send result is [%d]", result);
      } else {
        auto &globalSem = GlobalSem::getInstance();
        globalSem.waitSemBySemName(GlobalSem::loginLogout);
      }

      break;
    }
  }
  return ret;
}

bool CtpSender::ReqUserLogout() {
  bool ret = true;
  CThostFtdcUserLogoutField reqUserLogout = {0};
  auto &jsonCfg = utils::JsonConfig::getInstance();

  auto users = jsonCfg.getConfig("market", "User");
  for (auto &user : users) {
    const std::string userID = jsonCfg.getDeepConfig("users", (std::string)user, "UserID").get<std::string>();
    const std::string brokerID = jsonCfg.getDeepConfig("users", (std::string)user, "BrokerID").get<std::string>();
    strcpy(reqUserLogout.UserID, userID.c_str());
    strcpy(reqUserLogout.BrokerID, brokerID.c_str());

    int result = marketApi->ReqUserLogout(&reqUserLogout, nRequestID++);
    if (result != 0) {
      INFO_LOG("ReqUserLogout send result is [%d]", result);
    } else {
      auto &globalSem = GlobalSem::getInstance();
      if (globalSem.waitSemBySemName(GlobalSem::loginLogout, 3) != 0) {
        marketSpi->OnRspUserLogout();
      }
      release();
    }

    break;
  }
  return ret;
}

bool CtpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec) {
  int result = 0;
  int md_num = 0;

  if (nameVec.size() > 500) {
    WARNING_LOG("too much instruments to Subscription.");
    return result;
  }

  char **ppInstrumentID2 = new char *[5000];

  for (auto &neme : nameVec) {
    ppInstrumentID2[md_num] = const_cast<char *>(neme.ins.c_str());
    md_num++;
  }

  if (md_num > 0) {
    result = marketApi->SubscribeMarketData(ppInstrumentID2, md_num);
    if (result == 0) {
      INFO_LOG("Subscription request ......Send a success, total number: %d", md_num);
    } else {
      INFO_LOG("Subscription request ......Failed to send, error serial number=[%d]", result);
    }
  } else {
    INFO_LOG("no instrument need to Subscription.");
  }

  delete[] ppInstrumentID2;

  return result;
}

bool CtpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &nameVec) {
  int result = 0;
  int md_num = 0;

  if (nameVec.size() > 500) {
    WARNING_LOG("too much instruments to unSubscription.");
    return result;
  }

  char **ppInstrumentID2 = new char *[5000];

  for (auto &name : nameVec) {
    ppInstrumentID2[md_num] = const_cast<char *>(name.ins.c_str());
    md_num++;
  }

  if (md_num > 0) {
    result = marketApi->UnSubscribeMarketData(ppInstrumentID2, md_num);
    if (result == 0) {
      INFO_LOG("unSubscription request ......Send a success, total number: %d", md_num);
    } else {
      INFO_LOG(
          "unSubscription request ......Failed to send, error serial "
          "number=[%d]",
          result);
    }
  } else {
    INFO_LOG("no instrument need to unSubscription.");
  }

  delete[] ppInstrumentID2;

  return result;
};

bool CtpSender::ReqInstrumentInfo(const utils::InstrumtntID &ins) {
  INFO_LOG("not support.");
  return true;
}

bool CtpSender::LossConnection() { return (marketSpi != nullptr && marketSpi->frontDisconnected == true); }
