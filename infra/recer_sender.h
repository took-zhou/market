/*
 * recerSender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_RECERSENDER_H_
#define WORKSPACE_MARKET_INFRA_RECERSENDER_H_

#include "market/infra/sender/email_sender.h"
#include "market/infra/sender/itp_sender.h"
#include "market/infra/sender/proxy_sender.h"

#include "market/infra/recer/itp_recer.h"
#include "market/infra/recer/proxy_recer.h"

#include "common/self/dci/role.h"

struct Recer : ItpRecer, ProxyRecer {
  IMPL_ROLE(ItpRecer);
  IMPL_ROLE(ProxyRecer);
};

struct Sender : ItpSender, EmailSender, ProxySender {
  IMPL_ROLE(ItpSender);
  IMPL_ROLE(EmailSender);
  IMPL_ROLE(ProxySender);
};

struct RecerSender : Recer, Sender {
  RecerSender(){};
  RecerSender(const RecerSender &) = delete;
  RecerSender &operator=(const RecerSender &) = delete;
  static RecerSender &GetInstance() {
    static RecerSender instance;
    return instance;
  }

  IMPL_ROLE(Recer);
  IMPL_ROLE(Sender);
};

#endif /* WORKSPACE_MARKET_INFRA_RECERSENDER_H_ */
