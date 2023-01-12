/*
 * inner_sender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_INNERSENDER_H_
#define WORKSPACE_MARKET_INFRA_INNERSENDER_H_

#include <mutex>
#include <string>
#include "common/self/utils.h"

struct InnerSender {
 public:
  InnerSender();
  bool SendMsg(utils::ItpMsg &msg);

 private:
  void *pusher_{nullptr};
  std::mutex m_lock_;
};

#endif /* WORKSPACE_MARKET_INFRA_INNERSENDER_H_ */
