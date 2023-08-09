#ifndef WORKSPACE_MARKET_INFRA_SENDER_H_
#define WORKSPACE_MARKET_INFRA_SENDER_H_

#include <string>
#include <utility>
#include <vector>

#include "common/extern/curl/include/curl/curl.h"

struct EmailSender {
  struct UploadStatus {
    int bytes_read;
  };

 public:
  EmailSender();
  ~EmailSender();
  int Send(const std::string& subject = "", const std::string& body = "");

 private:
  std::string m_smtp_url_;
  std::string m_from_;
  std::string m_password_;
  std::vector<std::pair<std::string, std::string>> m_recvs_;
  std::vector<std::pair<std::string, std::string>> m_ccs_;
  std::string m_email_subject_;
  std::string m_email_body_;
  std::vector<std::string> m_attachments_;
};

#endif