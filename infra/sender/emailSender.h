/*
 * emailSender.h
 *
 *  Created on: 2022.04.28
 *      Author: Administrator
 */
#ifndef WORKSPACE_MARKET_INFRA_EMAILSENDER_H_
#define WORKSPACE_MARKET_INFRA_EMAILSENDER_H_
#include <string>

#include "common/extern/csmtp/CSmtp.h"

struct EmailSender {
  EmailSender();
  void send(const std::string &head, const std::string &msg);

  std::string mail_title;
  std::string mail_body;
};

#endif /* WORKSPACE_MARKET_INFRA_EMAILSENDER_H_ */
