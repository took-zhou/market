/*
 * recerSender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_RECERSENDER_H_
#define WORKSPACE_MARKET_INFRA_RECERSENDER_H_

#include "market/infra/sender/emailSender.h"
#include "market/infra/sender/itpSender.h"
#include "market/infra/sender/proxySender.h"

#include "market/infra/recer/itpRecer.h"
#include "market/infra/recer/proxyRecer.h"

#include "common/self/dci/Role.h"

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
  static RecerSender &getInstance() {
    static RecerSender instance;
    return instance;
  }

  IMPL_ROLE(Recer);
  IMPL_ROLE(Sender);
};

#endif /* WORKSPACE_MARKET_INFRA_RECERSENDER_H_ */
