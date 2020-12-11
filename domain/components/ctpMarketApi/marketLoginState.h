#ifndef RTW_HEADER_MarketLoginState_h_
#define RTW_HEADER_MarketLoginState_h_

#include <string.h>
#include "common/self/basetype.h"

enum LOGIN_STATE
{
    LOGIN_TIME = 1,
    LOGOUT_TIME = 2
};

typedef struct
{
    U8 is_active_MarketLoginState;
    U8 is_MarketLoginState;
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
    LOGIN_STATE status;
} STATE_OUTPUT;

typedef struct
{
    const S8 * volatile errorStatus;
}ERROR_STATUS;

class MarketLoginState
{
public:
    PARAM_INPUT input;

    STATE_OUTPUT output;

    // model initialize function
    void initialize();

    // model step function
    void step();

    // model update function
    static void update(MarketLoginState& login_state);

    MarketLoginState();

    ~MarketLoginState();

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
