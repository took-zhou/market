/*
 * recerSender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_TRADER_INFRA_RECERSENDER_H_
#define WORKSPACE_TRADER_INFRA_RECERSENDER_H_

#include "market/infra/sender/ctpSender.h"
#include "market/infra/sender/emailSender.h"
#include "market/infra/sender/interactSender.h"
#include "market/infra/sender/proxySender.h"

#include "market/infra/recer/ctpRecer.h"
#include "market/infra/recer/interactRecer.h"
#include "market/infra/recer/proxyRecer.h"

#include "common/self/dci/Role.h"

struct Recer : CtpRecer,
               InteractRecer,
               ProxyRecer

{
  bool init() {
    ROLE(CtpRecer).init();
    ROLE(InteractRecer).init();
    ROLE(ProxyRecer).init();
    return true;
  };
  IMPL_ROLE(CtpRecer);
  IMPL_ROLE(InteractRecer);
  IMPL_ROLE(ProxyRecer);
};

struct Sender : CtpSender, InteractSender, EmailSender, ProxySender {
  bool init() {
    ROLE(CtpSender).init();
    ROLE(InteractSender).init();
    ROLE(EmailSender).init();
    ROLE(ProxySender).init();
    return true;
  };
  IMPL_ROLE(CtpSender);
  IMPL_ROLE(InteractSender);
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
  bool init() {
    ROLE(Recer).init();
    ROLE(Sender).init();
    return true;
  }
  IMPL_ROLE(Recer);
  IMPL_ROLE(Sender);
};

#endif /* WORKSPACE_TRADER_INFRA_RECERSENDER_H_ */
