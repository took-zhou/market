/*
 * self_event.h
 *
 *  Created on: 2020年10月23日
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INTERFACE_SELFEVENT_H_
#define WORKSPACE_MARKET_INTERFACE_SELFEVENT_H_
#include <functional>
#include <map>
#include <string>
#include "common/self/utils.h"

struct SelfEvent {
 public:
  SelfEvent();
  void Handle(utils::ItpMsg &msg);
  void RegMsgFun();

  void SendEmailHandle(utils::ItpMsg &msg);

 private:
  std::map<std::string, std::function<void(utils::ItpMsg &msg)>> msg_func_map_;
};

#endif /* WORKSPACE_MARKET_INTERFACE_SELFEVENT_H_ */
