#ifndef RTW_HEADER_MarketLoginState_h_
#define RTW_HEADER_MarketLoginState_h_

#include <string.h>
#include "common/self/basetype.h"

enum MARKET_TIME_STATE
{
    LOGIN_TIME = 1,
    LOGOUT_TIME = 2
};

typedef struct
{
    U8 is_active_MarketTimeState;
    U8 is_MarketTimeState;
} DW;

typedef struct
{
    U32 now_mins;
    char loginTime[256];
    U32 day_login_mins;
    U32 day_logout_mins;
    U32 night_login_mins;
    U32 night_logout_mins;
    char loginMode[256];
} PARAM_INPUT;

typedef struct
{
    MARKET_TIME_STATE status;
} STATE_OUTPUT;

typedef struct
{
    const S8 * volatile errorStatus;
}ERROR_STATUS;

class MarketTimeState
{
public:
    PARAM_INPUT input;

    STATE_OUTPUT output;

    // model initialize function
    void initialize();

    // model step function
    void step();

    // model update function
    void update(void);

    MarketTimeState();

    ~MarketTimeState();

    ERROR_STATUS * getRTM();

private:
    DW rtDW = {0};

    ERROR_STATUS rtM;

    // private member function(s) for subsystem '<Root>'
    U32 isDuringDayLogoutTime(void);
    U32 isDuringNightLogoutTime(void);
    U32 isDuringDayLoginTime(void);
    U32 isDuringNightLoginTime(void);
    void determineLoginMode(void);
  };


#endif
