#ifndef SUBSCRIBE_MANAGER_H
#define SUBSCRIBE_MANAGER_H

#include <set>
#include <string>
#include <vector>

#include "common/self/utils.h"

struct InstrumentIDs {
  pthread_mutex_t sm_mutex;
  std::set<utils::InstrumtntID, utils::InstrumtntIDSortCriterion> instrument_ids;
};

struct SubscribeManager {
 public:
  SubscribeManager();
  void ReqInstrumentsFromLocal();
  void ReqInstrumrntFromControlPara();
  void ReqInstrumentsFromMarket();
  void ReqInstrumentsFromTrader();
  void SubscribeInstrument(std::vector<utils::InstrumtntID>& name_vec);
  void UnSubscribeInstrument(std::vector<utils::InstrumtntID>& name_vec);
  void UnSubscribeAll();

 private:
  const std::vector<utils::InstrumtntID> RemoveSubscribed(std::vector<utils::InstrumtntID>& name_vec);
  const std::vector<utils::InstrumtntID> RemoveUnsubscribed(std::vector<utils::InstrumtntID>& name_vec);
  InstrumentIDs subscribed_;
};

#endif