#include "market/infra/sender/email_sender.h"
#include "common/extern/log/log.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"

static char payload_text[1000];

static size_t PayloadSource(void *ptr, size_t size, size_t nmemb, void *userp) {
  struct EmailSender::UploadStatus *upload_ctx = (struct EmailSender::UploadStatus *)userp;
  const char *data;
  size_t room = size * nmemb;

  if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
    return 0;
  }

  data = &payload_text[upload_ctx->bytes_read];

  if (data) {
    size_t len = strlen(data);
    if (room < len) len = room;
    memcpy(ptr, data, len);
    upload_ctx->bytes_read += len;

    return len;
  }

  return 0;
}

EmailSender::EmailSender() {
  auto &json_cfg = utils::JsonConfig::GetInstance();
  m_smtp_url_ = json_cfg.GetConfig("emailbox", "server");
  m_from_ = json_cfg.GetConfig("emailbox", "sender");
  m_password_ = json_cfg.GetConfig("emailbox", "pwd");

  auto redipients = json_cfg.GetConfig("emailbox", "redipients");
  for (auto &redipient : redipients) {
    std::string recv_email = redipient.get<std::string>();
    std::string recv_name = utils::SplitString(redipient.get<std::string>(), "@")[0];
    m_recvs_.push_back(std::make_pair(recv_email, recv_name));
  }
}

EmailSender::~EmailSender() {
  m_recvs_.clear();
  m_ccs_.clear();
  m_attachments_.clear();
}

int EmailSender::Send(const std::string &subject, const std::string &body) {
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct UploadStatus upload_ctx;
  upload_ctx.bytes_read = 0;
  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, m_smtp_url_.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, m_from_.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, m_from_.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, m_password_.c_str());
    std::string recv_list = "";
    for (auto &item : m_recvs_) {
      recipients = curl_slist_append(recipients, item.first.c_str());
      recv_list += ";";
      recv_list += item.first;
    }
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    sprintf(payload_text, "To: %s\r\nFrom: chaodai DevOps team\r\nSubject: %s\r\n\r\n%s\r\n", recv_list.c_str(), subject.c_str(),
            body.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, PayloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      ERROR_LOG("curl_easy_perform() failed: %s.", curl_easy_strerror(res));
    }
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
  }

  return (int)res;
}