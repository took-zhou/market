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

EmailSender::EmailSender() {}

void EmailSender::Send(const char *head, const char *msg) {
  try {
    CSmtp mail;
    auto &json_cfg = utils::JsonConfig::GetInstance();
    std::string mail_server = json_cfg.GetConfig("emailbox", "server");
    mail.SetSMTPServer(mail_server.c_str());
    mail.SetSecurityType(NO_SECURITY);

    std::string mail_sender = json_cfg.GetConfig("emailbox", "sender");
    std::string mail_password = json_cfg.GetConfig("emailbox", "pwd");
    mail.SetLogin(mail_sender.c_str());
    mail.SetPassword(mail_password.c_str());

    mail.SetSenderName("chaodai DevOps team");
    mail.SetSenderMail(mail_sender.c_str());

    mail.SetCharSet("utf8");
    mail.SetXMailer("csmtp_v3");
    mail.SetXPriority(XPRIORITY_NORMAL);
    auto redipients = json_cfg.GetConfig("emailbox", "redipients");
    for (auto &redipient : redipients) {
      mail.AddRecipient(((std::string)redipient).c_str());
    }
    mail.SetSubject(head);
    mail.AddMsgLine(msg);
    mail.Send();  // Wow! Send mail!
    mail.ClearMessage();

    INFO_LOG("##### SEND MAIL #####");
  } catch (ECSmtp e) {
    ERROR_LOG("- Error: :%s.", e.GetErrorText().c_str());
  }
}
