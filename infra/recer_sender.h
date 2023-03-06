/*
 * recerSender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_RECERSENDER_H_
#define WORKSPACE_MARKET_INFRA_RECERSENDER_H_

#include "market/infra/recer/inner_recer.h"
#include "market/infra/recer/proxy_recer.h"
#include "market/infra/sender/direct_sender.h"
#include "market/infra/sender/email_sender.h"
#include "market/infra/sender/inner_sender.h"
#include "market/infra/sender/itp_sender.h"
#include "market/infra/sender/proxy_sender.h"


#include <thread>
#include "common/self/dci/role.h"

struct Recer : InnerRecer, ProxyRecer {
  IMPL_ROLE(InnerRecer);
  IMPL_ROLE(ProxyRecer);
};

struct Sender : ItpSender, EmailSender, ProxySender, InnerSender, DirectSender {
  IMPL_ROLE(ItpSender);
  IMPL_ROLE(EmailSender);
  IMPL_ROLE(ProxySender);
  IMPL_ROLE(InnerSender);
  IMPL_ROLE(DirectSender);
};

struct RecerSender : Recer, Sender {
  RecerSender(){};
  RecerSender(const RecerSender &) = delete;
  RecerSender &operator=(const RecerSender &) = delete;
  static RecerSender &GetInstance() {
    static RecerSender instance;
    return instance;
  }

  void Run() {
    while (1) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  IMPL_ROLE(Recer);
  IMPL_ROLE(Sender);
};

#endif /* WORKSPACE_MARKET_INFRA_RECERSENDER_H_ */
