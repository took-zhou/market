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
  void ReqInstrumentsFromApi();
  void SubscribeInstrument(std::vector<utils::InstrumtntID>& name_vec, int request_id = 0);
  void UnSubscribeInstrument(std::vector<utils::InstrumtntID>& name_vec, int request_id = 0);
  void UnSubscribeAll();
  void EraseAllSubscribed();

 private:
  void AddSubscribed(std::vector<utils::InstrumtntID>& name_vec);
  void RemoveSubscribed(std::vector<utils::InstrumtntID>& name_vec);
  InstrumentIDs subscribed_;
};

#endif