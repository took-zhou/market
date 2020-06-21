//
// File: MarketLoginControl.cpp
//
// Code generated for Simulink model 'MarketLoginControl'.
//
// Model version                  : 1.29
// Simulink Coder version         : 9.0 (R2018b) 24-May-2018
// C/C++ source code generated on : Sun Jun 21 23:14:08 2020
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives:
//    1. Execution efficiency
//    2. RAM efficiency
// Validation result: Not run
//
#include "MarketLoginControl.h"

// Named constants for Chart: '<Root>/MarketLoginControl'
#define IN_day_login                   ((uint8_T)1U)
#define IN_day_logout                  ((uint8_T)2U)
#define IN_init_sts                    ((uint8_T)3U)
#define IN_night_login                 ((uint8_T)4U)
#define IN_night_logout                ((uint8_T)5U)
#define login                          ((uint8_T)1U)
#define logout                         ((uint8_T)2U)

// Function for Chart: '<Root>/MarketLoginControl'
uint32_T MarketLoginControlModelClass::isDuringDayLogoutTime(void)
{
  uint32_T out;
  out = 0U;

  // Inport: '<Root>/loginTime' incorporates:
  //   Inport: '<Root>/day_logout_mins'
  //   Inport: '<Root>/loginMode'
  //   Inport: '<Root>/night_login_mins'
  //   Inport: '<Root>/now_mins'

  if ((strcmp(rtU.loginTime, "night") != 0) && (strcmp(rtU.loginMode, "normal") ==
       0) && ((rtU.day_logout_mins < rtU.now_mins) && (rtU.now_mins <
        rtU.night_login_mins))) {
    out = 1U;
  }

  // End of Inport: '<Root>/loginTime'
  return out;
}

// Function for Chart: '<Root>/MarketLoginControl'
uint32_T MarketLoginControlModelClass::isDuringNightLogoutTime(void)
{
  uint32_T out;
  out = 0U;

  // Inport: '<Root>/loginTime' incorporates:
  //   Inport: '<Root>/day_login_mins'
  //   Inport: '<Root>/loginMode'
  //   Inport: '<Root>/night_logout_mins'
  //   Inport: '<Root>/now_mins'

  if ((strcmp(rtU.loginTime, "day") != 0) && (strcmp(rtU.loginMode, "normal") ==
       0) && ((rtU.night_logout_mins < rtU.now_mins) && (rtU.now_mins <
        rtU.day_login_mins))) {
    out = 1U;
  }

  // End of Inport: '<Root>/loginTime'
  return out;
}

// Function for Chart: '<Root>/MarketLoginControl'
uint32_T MarketLoginControlModelClass::isDuringDayLoginTime(void)
{
  uint32_T out;
  out = 0U;

  // Inport: '<Root>/loginTime' incorporates:
  //   Inport: '<Root>/day_login_mins'
  //   Inport: '<Root>/day_logout_mins'
  //   Inport: '<Root>/loginMode'
  //   Inport: '<Root>/now_mins'

  if ((strcmp(rtU.loginTime, "night") != 0) && (strcmp(rtU.loginMode, "normal") ==
       0) && ((rtU.day_login_mins <= rtU.now_mins) && (rtU.now_mins <=
        rtU.day_logout_mins))) {
    out = 1U;
  }

  // End of Inport: '<Root>/loginTime'
  return out;
}

// Function for Chart: '<Root>/MarketLoginControl'
uint32_T MarketLoginControlModelClass::isDuringNightLoginTime(void)
{
  uint32_T out;
  out = 0U;

  // Inport: '<Root>/loginTime' incorporates:
  //   Inport: '<Root>/loginMode'
  //   Inport: '<Root>/night_login_mins'
  //   Inport: '<Root>/night_logout_mins'
  //   Inport: '<Root>/now_mins'

  if ((strcmp(rtU.loginTime, "day") != 0) && (strcmp(rtU.loginMode, "normal") ==
       0) && ((rtU.night_login_mins <= rtU.now_mins) || (rtU.now_mins <=
        rtU.night_logout_mins))) {
    out = 1U;
  }

  // End of Inport: '<Root>/loginTime'
  return out;
}

// Function for Chart: '<Root>/MarketLoginControl'
void MarketLoginControlModelClass::determineLoginMode(void)
{
  // Inport: '<Root>/loginMode'
  if (strcmp(rtU.loginMode, "normal") == 0) {
    // Outport: '<Root>/status'
    rtY.status = logout;
  } else {
    // Outport: '<Root>/status'
    rtY.status = login;
  }

  // End of Inport: '<Root>/loginMode'
}

// Model step function
void MarketLoginControlModelClass::step()
{
  // Chart: '<Root>/MarketLoginControl' incorporates:
  //   Inport: '<Root>/day_login_mins'
  //   Inport: '<Root>/day_logout_mins'
  //   Inport: '<Root>/loginTime'
  //   Inport: '<Root>/night_login_mins'
  //   Inport: '<Root>/night_logout_mins'
  //   Inport: '<Root>/now_mins'

  if (rtDW.is_active_c3_MarketLoginControl == 0U) {
    rtDW.is_active_c3_MarketLoginControl = 1U;
    rtDW.is_c3_MarketLoginControl = IN_init_sts;
    determineLoginMode();
  } else {
    switch (rtDW.is_c3_MarketLoginControl) {
     case IN_day_login:
      if (rtU.now_mins == rtU.day_logout_mins) {
        rtDW.is_c3_MarketLoginControl = IN_day_logout;

        // Outport: '<Root>/status'
        rtY.status = logout;
      }
      break;

     case IN_day_logout:
      if ((strcmp(rtU.loginTime, "day") != 0) && (rtU.now_mins ==
           rtU.night_login_mins)) {
        rtDW.is_c3_MarketLoginControl = IN_night_login;

        // Outport: '<Root>/status'
        rtY.status = login;
      } else {
        if ((strcmp(rtU.loginTime, "day") == 0) && (rtU.now_mins ==
             rtU.day_login_mins)) {
          rtDW.is_c3_MarketLoginControl = IN_day_login;

          // Outport: '<Root>/status'
          rtY.status = login;
        }
      }
      break;

     case IN_init_sts:
      if (isDuringDayLogoutTime() != 0U) {
        rtDW.is_c3_MarketLoginControl = IN_day_logout;

        // Outport: '<Root>/status'
        rtY.status = logout;
      } else if (isDuringNightLogoutTime() != 0U) {
        rtDW.is_c3_MarketLoginControl = IN_night_logout;

        // Outport: '<Root>/status'
        rtY.status = logout;
      } else if (isDuringDayLoginTime() != 0U) {
        rtDW.is_c3_MarketLoginControl = IN_day_login;

        // Outport: '<Root>/status'
        rtY.status = login;
      } else {
        if (isDuringNightLoginTime() != 0U) {
          rtDW.is_c3_MarketLoginControl = IN_night_login;

          // Outport: '<Root>/status'
          rtY.status = login;
        }
      }
      break;

     case IN_night_login:
      if (rtU.now_mins == rtU.night_logout_mins) {
        rtDW.is_c3_MarketLoginControl = IN_night_logout;

        // Outport: '<Root>/status'
        rtY.status = logout;
      }
      break;

     default:
      if ((strcmp(rtU.loginTime, "night") != 0) && (rtU.now_mins ==
           rtU.day_login_mins)) {
        rtDW.is_c3_MarketLoginControl = IN_day_login;

        // Outport: '<Root>/status'
        rtY.status = login;
      } else {
        if ((strcmp(rtU.loginTime, "night") == 0) && (rtU.now_mins ==
             rtU.night_login_mins)) {
          rtDW.is_c3_MarketLoginControl = IN_night_login;

          // Outport: '<Root>/status'
          rtY.status = login;
        }
      }
      break;
    }
  }

  // End of Chart: '<Root>/MarketLoginControl'
}

// Model initialize function
void MarketLoginControlModelClass::initialize()
{
  // (no initialization code required)
}

// Constructor
MarketLoginControlModelClass::MarketLoginControlModelClass()
{
  // Currently there is no constructor body generated.
}

// Destructor
MarketLoginControlModelClass::~MarketLoginControlModelClass()
{
  // Currently there is no destructor body generated.
}

// Real-Time Model get method
RT_MODEL * MarketLoginControlModelClass::getRTM()
{
  return (&rtM);
}

//
// File trailer for generated code.
//
// [EOF]
//
