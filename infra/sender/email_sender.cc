/*
 * emailSender.cpp
 *
 *  Created on: 2022.04.28
 *      Author: Administrator
 */
#include <iostream>
#include <thread>

#include "common/extern/csmtp/csmtp.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "market/infra/sender/email_sender.h"

EmailSender::EmailSender() {
  auto &json_cfg = utils::JsonConfig::GetInstance();

  auto redipients = json_cfg.GetConfig("emailbox", "redipients");
  for (auto &redipient : redipients) {
    mail_.AddRecipient(((std::string)redipient).c_str());
  }

  std::string mail_server = json_cfg.GetConfig("emailbox", "server");
  mail_.SetSMTPServer(mail_server.c_str());
  mail_.SetSecurityType(NO_SECURITY);

  std::string mail_sender = json_cfg.GetConfig("emailbox", "sender");
  std::string mail_password = json_cfg.GetConfig("emailbox", "pwd");
  mail_.SetLogin(mail_sender.c_str());
  mail_.SetPassword(mail_password.c_str());

  mail_.SetSenderName("chaodai DevOps team");
  mail_.SetSenderMail(mail_sender.c_str());

  mail_.SetCharSet("utf8");
  mail_.SetXMailer("csmtp_v3");
  mail_.SetXPriority(XPRIORITY_NORMAL);
}

void EmailSender::Send(const char *head, const char *msg) {
  try {
    mail_.SetSubject(head);
    mail_.AddMsgLine(msg);
    mail_.Send();  // Wow! Send mail!
    mail_.ClearMessage();

    INFO_LOG("##### SEND MAIL #####");
  } catch (ECSmtp e) {
    ERROR_LOG("- Error: :%s.", e.GetErrorText().c_str());
  }
}
