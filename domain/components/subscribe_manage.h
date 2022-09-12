#ifndef SUBSCRIBE_MANAGER_H
#define SUBSCRIBE_MANAGER_H

#include <set>
#include <string>
#include <vector>

#include "common/self/utils.h"

struct instrumentIDs {
  pthread_mutex_t sm_mutex;
  std::set<utils::InstrumtntID, utils::InstrumtntIDSortCriterion> instrumentIDs;
};

struct SubscribeManager {
 public:
  SubscribeManager();
  void reqInstrumentsFromLocal();
  void reqInstrumrntFromControlPara();
  void reqInstrumentsFromMarket();
  void reqInstrumentsFromTrader();
  void subscribeInstrument(std::vector<utils::InstrumtntID>& nameVec);
  void unSubscribeInstrument(std::vector<utils::InstrumtntID>& nameVec);
  void unSubscribeAll();

 private:
  const std::vector<utils::InstrumtntID> remove_subscribed(std::vector<utils::InstrumtntID>& nameVec);
  const std::vector<utils::InstrumtntID> remove_unsubscribed(std::vector<utils::InstrumtntID>& nameVec);
  instrumentIDs subscribed;
};

#endif