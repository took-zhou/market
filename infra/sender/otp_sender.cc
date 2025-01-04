#include "market/infra/sender/otp_sender.h"
#include <cstddef>
#include <string>
#include <thread>
#include "common/extern/log/log.h"
#include "common/extern/otp/inc/mds_api/mds_async_api.h"
#include "common/self/file_util.h"
#include "common/self/utils.h"
#include "market/infra/recer/otp_recer.h"
#include "mds_api/mds_api.h"
#include "mds_global/mds_base_model.h"
#include "mds_global/mds_mkt_packets.h"

OtpMarketSpi *OtpSender::market_spi;

static int32 ApiAsyncConnect(MdsAsyncApiChannelT *async_channel, void *callback);
static int32 ApiAsyncDisconnect(MdsAsyncApiChannelT *async_channel, void *callback);
static int32 QryStockStaticInfo(MdsApiSessionInfoT *session, SMsgHeadT *head, void *item, MdsQryCursorT *cursor, void *callback);
static int32 HandleMsg(MdsApiSessionInfoT *session, SMsgHeadT *head, void *item, void *callback);

OtpSender::OtpSender(void) {}

OtpSender::~OtpSender(void) {}

bool OtpSender::Init(void) {
  bool out = true;
  if (!is_init_) {
    INFO_LOG("begin otp market api init");
    if (!__MdsApi_CheckApiVersion()) {
      ERROR_LOG("api version error, version[%s], libversion[%s]", MDS_APPL_VER_ID, MdsApi_GetApiVersion());
    } else {
      INFO_LOG("api version: %s", MdsApi_GetApiVersion());
    }
    if (!MdsApi_InitLoggerDirect("file", "FATAL", "./api.log", 0, 0)) {
      ERROR_LOG("init oes log fail!");
    }
    market_spi = new OtpMarketSpi();

    MdsAsyncApiContextParamsT context_params = {DEFAULT_MDSAPI_ASYNC_CONTEXT_PARAMS};
    context_params.asyncQueueSize = 100000;
    context_params.isHugepageAble = TRUE;
    context_params.isAsyncCallbackAble = FALSE;
    context_params.isBusyPollAble = TRUE;
    context_params.isPreconnectAble = TRUE;
    context_params.isBuiltinQueryable = TRUE;
    async_context_ = MdsAsyncApi_CreateContextSimple2(NULL, NULL, &context_params);
    if (!async_context_) {
      ERROR_LOG("failed to create an asynchronous api runtime environment.");
      out = false;
    }
    auto &json_cfg = utils::JsonConfig::GetInstance();
    auto users = json_cfg.GetConfig("market", "User");
    for (auto &user : users) {
      MdsApiRemoteCfgT remote_cfg = {NULLOBJ_MDSAPI_REMOTE_CFG};
      auto tcp_server = json_cfg.GetDeepConfig("users", static_cast<std::string>(user), "FrontMdTcpServer").get<std::string>();
      auto qry_server = json_cfg.GetDeepConfig("users", static_cast<std::string>(user), "FrontMdQryServer").get<std::string>();
      auto user_id = json_cfg.GetDeepConfig("users", user, "UserID").get<std::string>();
      auto password = json_cfg.GetDeepConfig("users", user, "Password").get<std::string>();
      remote_cfg.addrCnt = MdsApi_ParseAddrListString(tcp_server.c_str(), remote_cfg.addrList, GENERAL_CLI_MAX_REMOTE_CNT);
      strcpy(remote_cfg.username, user_id.c_str());
      strcpy(remote_cfg.password, password.c_str());
      remote_cfg.heartBtInt = 30;
      async_channel_ = MdsAsyncApi_AddChannel(async_context_, nullptr, &remote_cfg, nullptr, HandleMsg, market_spi, ApiAsyncConnect,
                                              market_spi, ApiAsyncDisconnect, market_spi);
      if (!async_channel_) {
        ERROR_LOG("failed to add a return channel.");
        out = false;
      }
      remote_cfg.addrCnt = MdsApi_ParseAddrListString(qry_server.c_str(), remote_cfg.addrList, GENERAL_CLI_MAX_REMOTE_CNT);
      if (!MdsAsyncApi_SetBuiltinQueryChannelCfg(async_context_, &remote_cfg)) {
        ERROR_LOG("failed to add a return channel.");
        out = false;
      }

      is_init_ = true;
      break;
    }
  }
  return out;
}

bool OtpSender::ReqUserLogin(void) {
  INFO_LOG("login time, is going to login.");
  bool ret = true;
  if (!Init()) {
    Release();
    ret = false;
  } else {
    if (!MdsAsyncApi_Start(async_context_)) {
      ERROR_LOG("failed to start the asynchronous api thread!");
      ret = FALSE;
    }
    UpdateInstrumentInfoFromMarket();

    std::string result = "success";
    market_spi->OnRspUserLogin(&result);
    INFO_LOG("login otp ok!");
  }
  return ret;
}

bool OtpSender::ReqUserLogout() {
  INFO_LOG("logout time, is going to logout.");

  Release();

  std::string result = "success";
  market_spi->OnRspUserLogout(&result);
  return true;
}

bool OtpSender::Release() {
  INFO_LOG("is going to release quote api.");

  if (async_context_ != nullptr) {
    MdsAsyncApi_Stop(async_context_);
    MdsAsyncApi_ReleaseContext(async_context_);
    async_context_ = nullptr;
  }

  // 释放UserSpi实例
  if (market_spi != nullptr) {
    delete market_spi;
    market_spi = nullptr;
  }
  is_init_ = false;

  return true;
}

void OtpSender::UpdateInstrumentInfoFromMarket() {
  int32_t ret = 0;
  MdsQryStockStaticInfoListFilterT qry_filter = {NULLOBJ_MDS_QRY_STOCK_STATIC_INFO_LIST_FILTER};
  qry_filter.exchId = 0;
  qry_filter.oesSecurityType = MDS_MD_PRODUCT_TYPE_STOCK;
  qry_filter.subSecurityType = 0;
  ret = MdsAsyncApi_QueryStockStaticInfoList(async_context_, nullptr, nullptr, &qry_filter, QryStockStaticInfo, market_spi);
  if (ret < 0 && ret != GENERAL_CLI_RTCODE_BREAK) {
    ERROR_LOG("req instrumentinfo fail.");
  }
}

bool OtpSender::SubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
  int result = true;
  if (name_vec.size() > 500) {
    WARNING_LOG("too much instruments to un subscription.");
    return result;
  }

  unsigned int count = 0;
  std::string shse_instruments = "";
  std::string szse_instruments = "";

  for (auto &item : name_vec) {
    if (item.exch == "SHSE") {
      shse_instruments += item.ins;
      shse_instruments += "|";
    } else if (item.exch == "SZSE") {
      szse_instruments += item.ins;
      szse_instruments += "|";
    }
    count++;
    INFO_LOG("exch: %s ins: %s", item.exch.c_str(), item.ins.c_str());
  }
  if (shse_instruments.size() > 0) {
    INFO_LOG("shse instruments %s ", shse_instruments.c_str());
    result = MdsAsyncApi_SubscribeByString(async_channel_, shse_instruments.c_str(), "|", MDS_EXCH_SSE, MDS_MD_PRODUCT_TYPE_STOCK,
                                           MDS_SUB_MODE_APPEND, MDS_SUB_DATA_TYPE_L2_SNAPSHOT);
  }
  if (szse_instruments.size() > 0) {
    INFO_LOG("szse instruments %s ", szse_instruments.c_str());
    result = MdsAsyncApi_SubscribeByString(async_channel_, szse_instruments.c_str(), "|", MDS_EXCH_SZSE, MDS_MD_PRODUCT_TYPE_STOCK,
                                           MDS_SUB_MODE_APPEND, MDS_SUB_DATA_TYPE_L2_SNAPSHOT);
  }

  if (count == 0) {
    INFO_LOG("no instrument need to Subscription.");
  } else {
    if (static_cast<bool>(result)) {
      INFO_LOG("subscription request ......send a success, total number: %d", count);
    } else {
      ERROR_LOG("subscribe market data fail, error code[%d]", result);
    }
  }

  return true;
}

bool OtpSender::UnSubscribeMarketData(std::vector<utils::InstrumtntID> const &name_vec, int request_id) {
  int result = true;
  if (name_vec.size() > 500) {
    WARNING_LOG("too much instruments to un subscription.");
    return result;
  }

  unsigned int count = 0;
  std::string shse_instruments = "";
  std::string szse_instruments = "";

  for (auto &item : name_vec) {
    if (item.exch == "SHSE") {
      shse_instruments += item.ins;
      shse_instruments += "|";
    } else if (item.exch == "SZSE") {
      szse_instruments += item.ins;
      szse_instruments += "|";
    }
    count++;
  }
  if (shse_instruments.size() > 0) {
    result = MdsAsyncApi_SubscribeByString(async_channel_, shse_instruments.c_str(), "|", MDS_EXCH_SSE, MDS_MD_PRODUCT_TYPE_STOCK,
                                           MDS_SUB_MODE_DELETE, MDS_SUB_DATA_TYPE_L2_SNAPSHOT);
  }
  if (szse_instruments.size() > 0) {
    result = MdsAsyncApi_SubscribeByString(async_channel_, szse_instruments.c_str(), "|", MDS_EXCH_SZSE, MDS_MD_PRODUCT_TYPE_STOCK,
                                           MDS_SUB_MODE_DELETE, MDS_SUB_DATA_TYPE_L2_SNAPSHOT);
  }

  if (count == 0) {
    INFO_LOG("no instrument need to un subscription.");
  } else {
    if (static_cast<bool>(result)) {
      INFO_LOG("un subscription request ......send a success, total number: %d", count);
    } else {
      ERROR_LOG("un subscription fail, error code[%d]", result);
    }
  }

  return true;
}

bool OtpSender::LossConnection() { return false; }

static int32 HandleMsg(MdsApiSessionInfoT *session, SMsgHeadT *head, void *item, void *callback) {
  auto *rsp_msg = static_cast<MdsMktRspMsgBodyT *>(item);
  auto *market_spi = static_cast<OtpMarketSpi *>(callback);

  switch (head->msgId) {
    case MDS_MSGTYPE_L2_MARKET_DATA_SNAPSHOT:
      market_spi->OnDepthMarketData(&rsp_msg->mktDataSnapshot);
      break;
    default:
      break;
  }

  return 0;
}

static int32 ApiAsyncConnect(MdsAsyncApiChannelT *async_channel, void *callback) {
  auto *market_spi = static_cast<OtpMarketSpi *>(callback);
  std::string result = "success";
  market_spi->OnRspUserLogin(&result);
  return 0;
}

static int32 ApiAsyncDisconnect(MdsAsyncApiChannelT *async_channel, void *callback) {
  auto *market_spi = static_cast<OtpMarketSpi *>(callback);
  std::string result = "success";
  market_spi->OnRspUserLogout(&result);
  return 0;
}

static int32 QryStockStaticInfo(MdsApiSessionInfoT *session, SMsgHeadT *head, void *item, MdsQryCursorT *cursor, void *callback) {
  auto *static_info = static_cast<MdsStockStaticInfoT *>(item);
  auto *market_spi = static_cast<OtpMarketSpi *>(callback);
  market_spi->OnRspStockStaticInfo(static_info, cursor->isEnd);
  return 0;
}