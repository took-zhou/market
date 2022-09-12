/*
 * ctpSender.cpp
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#include "market/infra/sender/ctp_sender.h"
#include <string>
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/semaphore.h"
#include "common/self/utils.h"
#include "market/infra/recer/ctp_recer.h"

CThostFtdcMdApi *CtpSender::kMarketApi;
CtpMarketSpi *CtpSender::kMarketSpi;

CtpSender::CtpSender(void) { ; }

bool CtpSender::Init(void) {
  bool out = true;
  if (is_init == false) {
    auto &jsonCfg = utils::JsonConfig::getInstance();
    auto users = jsonCfg.get_config("market", "User");
    for (auto &user : users) {
      con_path = jsonCfg.get_config("market", "con_path").get<std::string>() + "/" + (std::string)user + "/";
      utils::CreatFolder(con_path);
      kMarketApi = CThostFtdcMdApi::CreateFtdcMdApi(con_path.c_str(), true, true);
      INFO_LOG("ctp version: %s.", kMarketApi->GetApiVersion());

      kMarketSpi = new CtpMarketSpi();
      kMarketApi->RegisterSpi(kMarketSpi);

      std::string frontaddr = jsonCfg.get_deep_config("users", (std::string)user, "FrontMdAddr").get<std::string>();

      kMarketApi->RegisterFront(const_cast<char *>(frontaddr.c_str()));
      kMarketApi->Init();

      auto &globalSem = GlobalSem::getInstance();
      if (globalSem.WaitSemBySemName(GlobalSem::kLoginLogout, 3)) {
        out = false;
        ERROR_LOG("market init fail.");
      } else {
        out = true;
        INFO_LOG("market init ok.");
      }
      break;
    }
    is_init = true;
  }

  return out;
}

bool CtpSender::Release() {
  INFO_LOG("Is going to release kMarketApi.");
  kMarketApi->Release();

  // 释放UserSpi实例
  if (kMarketSpi) {
    delete kMarketSpi;
    kMarketSpi = NULL;
  }

  is_init = false;
}

bool CtpSender::ReqUserLogin() {
  bool ret = true;
  if (Init() == false) {
    ret = false;
    Release();
  } else {
    CThostFtdcReqUserLoginField reqUserLogin = {0};
    auto &jsonCfg = utils::JsonConfig::getInstance();

    auto users = jsonCfg.get_config("market", "User");
    for (auto &user : users) {
      const std::string userID = jsonCfg.get_deep_config("users", (std::string)user, "UserID");
      const std::string brokerID = jsonCfg.get_deep_config("users", (std::string)user, "BrokerID");
      const std::string passWord = jsonCfg.get_deep_config("users", (std::string)user, "Password");
      strcpy(reqUserLogin.BrokerID, brokerID.c_str());
      strcpy(reqUserLogin.UserID, userID.c_str());
      strcpy(reqUserLogin.Password, passWord.c_str());

      int result = kMarketApi->ReqUserLogin(&reqUserLogin, request_id++);
      if (result != 0) {
        INFO_LOG("ReqUserLogin send result is [%d]", result);
      } else {
        auto &globalSem = GlobalSem::getInstance();
        globalSem.WaitSemBySemName(GlobalSem::kLoginLogout);
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

  auto users = jsonCfg.get_config("market", "User");
  for (auto &user : users) {
    const std::string userID = jsonCfg.get_deep_config("users", (std::string)user, "UserID").get<std::string>();
    const std::string brokerID = jsonCfg.get_deep_config("users", (std::string)user, "BrokerID").get<std::string>();
    strcpy(reqUserLogout.UserID, userID.c_str());
    strcpy(reqUserLogout.BrokerID, brokerID.c_str());

    int result = kMarketApi->ReqUserLogout(&reqUserLogout, request_id++);
    if (result != 0) {
      INFO_LOG("ReqUserLogout send result is [%d]", result);
    } else {
      auto &globalSem = GlobalSem::getInstance();
      if (globalSem.WaitSemBySemName(GlobalSem::kLoginLogout, 3) != 0) {
        kMarketSpi->OnRspUserLogout();
      }
      Release();
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
    result = kMarketApi->SubscribeMarketData(ppInstrumentID2, md_num);
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
    result = kMarketApi->UnSubscribeMarketData(ppInstrumentID2, md_num);
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

bool CtpSender::LossConnection() { return (kMarketSpi != nullptr && kMarketSpi->front_disconnected == true); }
