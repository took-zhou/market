/*
 * emailSender.cpp
 *
 *  Created on: 2022.04.28
 *      Author: Administrator
 */
#include <iostream>
#include <thread>

#include "common/extern/csmtp/CSmtp.h"
#include "common/extern/log/log.h"
#include "common/self/fileUtil.h"
#include "market/infra/sender/emailSender.h"

EmailSender::EmailSender() {}

void EmailSender::send(const std::string &head, const std::string &msg) {
  mail_title = head;
  mail_body = msg;

  auto sendfunc = [=]() {
    try {
      CSmtp mail;

      auto &jsonCfg = utils::JsonConfig::getInstance();

      auto redipients = jsonCfg.getConfig("emailbox", "redipients");
      for (auto &redipient : redipients) {
        mail.AddRecipient(((std::string)redipient).c_str());
      }

      std::string mail_server = jsonCfg.getConfig("emailbox", "server");
      mail.SetSMTPServer(mail_server.c_str());
      mail.SetSecurityType(NO_SECURITY);

      std::string mail_sender = jsonCfg.getConfig("emailbox", "sender");
      std::string mail_password = jsonCfg.getConfig("emailbox", "pwd");
      mail.SetLogin(mail_sender.c_str());
      mail.SetPassword(mail_password.c_str());

      mail.SetSenderName("chaodai DevOps team");
      mail.SetSenderMail(mail_sender.c_str());

      mail.SetCharSet("utf8");
      mail.SetXMailer("csmtp_v3");
      mail.SetXPriority(XPRIORITY_NORMAL);

      mail.SetSubject(mail_title.c_str());
      mail.AddMsgLine(mail_body.c_str());

      mail.Send();  // Wow! Send mail!
      // INFO_LOG("mail last output: %s", mail.GetLastOutput());

      // Clear mail content and last output, then you can fill
      // with a new mail and send it.
      mail.ClearMessage();

      INFO_LOG("##### SEND MAIL #####");
    } catch (ECSmtp e) {
      ERROR_LOG("- Error: :%s.", e.GetErrorText().c_str());
    }
  };
  std::thread(sendfunc).detach();
}
