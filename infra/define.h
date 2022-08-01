/*
 * define.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_DEFINE_H_
#define WORKSPACE_MARKET_INFRA_DEFINE_H_

#include <string>

enum struct EventType { Ctp_Event = 0, Market_Event = 1, Trader_Event = 2, Strategy_Event = 3, Manage_Event = 4, INVALID = 5 };

struct MsgStruct {
  std::string sessionName{""};
  std::string msgName{""};
  std::string pbMsg{""};
  void *ctpMsg{nullptr};
  bool isValid() { return sessionName != std::string("") and msgName != std::string(""); }
};

#endif /* WORKSPACE_MARKET_INFRA_DEFINE_H_ */
