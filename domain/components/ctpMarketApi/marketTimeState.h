#ifndef RTW_HEADER_MarketLoginState_h_
#define RTW_HEADER_MarketLoginState_h_

#include <string.h>
#include "common/self/basetype.h"

enum MARKET_TIME_STATE { RESERVE = 0, LOGIN_TIME = 1, LOGOUT_TIME = 2 };

enum LOGIN_STATE { IN_day_login = 1, IN_day_logout = 2, IN_init_sts = 3, IN_night_login = 4, IN_night_logout = 5 };

typedef struct {
  U8 is_active_MarketTimeState;
  U8 is_MarketTimeState;
} DW;

typedef struct {
  U32 now_mins;
  U32 day_login_mins;
  U32 day_logout_mins;
  U32 night_login_mins;
  U32 night_logout_mins;
} PARAM_INPUT;

typedef struct {
  MARKET_TIME_STATE status;
} STATE_OUTPUT;

typedef struct {
  const S8* volatile errorStatus;
} ERROR_STATUS;

class MarketTimeState {
 public:
  PARAM_INPUT input;

  STATE_OUTPUT output;

  // model step function
  void step();

  // model update function
  void update(void);

  void set_time_state(int command);

  MarketTimeState();

  ~MarketTimeState();

  ERROR_STATUS* getRTM();

  DW rtDW = {0};

 private:
  ERROR_STATUS rtM;

  // private member function(s) for subsystem '<Root>'
  U32 isDuringDayLogoutTime(void);
  U32 isDuringNightLogoutTime(void);
  U32 isDuringDayLoginTime(void);
  U32 isDuringNightLoginTime(void);

  MARKET_TIME_STATE time_state = RESERVE;
};

#endif
