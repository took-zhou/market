//
// File: MarketLoginControl.h
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
#ifndef RTW_HEADER_MarketLoginControl_h_
#define RTW_HEADER_MarketLoginControl_h_
#include <string.h>
#ifndef MarketLoginControl_COMMON_INCLUDES_
# define MarketLoginControl_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 // MarketLoginControl_COMMON_INCLUDES_

// Macros for accessing real-time model data structure
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

// Forward declaration for rtModel
typedef struct tag_RTM RT_MODEL;

// Block signals and states (default storage) for system '<Root>'
typedef struct {
  uint8_T is_active_c3_MarketLoginControl;// '<Root>/MarketLoginControl'
  uint8_T is_c3_MarketLoginControl;    // '<Root>/MarketLoginControl'
} DW;

// External inputs (root inport signals with default storage)
typedef struct {
  uint32_T now_mins;                   // '<Root>/now_mins'
  char_T loginTime[256];               // '<Root>/loginTime'
  uint32_T day_login_mins;             // '<Root>/day_login_mins'
  uint32_T day_logout_mins;            // '<Root>/day_logout_mins'
  uint32_T night_login_mins;           // '<Root>/night_login_mins'
  uint32_T night_logout_mins;          // '<Root>/night_logout_mins'
  char_T loginMode[256];               // '<Root>/loginMode'
} ExtU;

// External outputs (root outports fed by signals with default storage)
typedef struct {
  uint8_T status;                      // '<Root>/status'
} ExtY;

// Real-time Model Data Structure
struct tag_RTM {
  const char_T * volatile errorStatus;
};

// Class declaration for model MarketLoginControl
class MarketLoginControlModelClass {
  // public data and function members
 public:
  // External inputs
  ExtU rtU;

  // External outputs
  ExtY rtY;

  // model initialize function
  void initialize();

  // model step function
  void step();

  // Constructor
  MarketLoginControlModelClass();

  // Destructor
  ~MarketLoginControlModelClass();

  // Real-Time Model get method
  RT_MODEL * getRTM();

  // private data and function members
 private:
  // Block signals and states
  DW rtDW;

  // Real-Time Model
  RT_MODEL rtM;

  // private member function(s) for subsystem '<Root>'
  uint32_T isDuringDayLogoutTime(void);
  uint32_T isDuringNightLogoutTime(void);
  uint32_T isDuringDayLoginTime(void);
  uint32_T isDuringNightLoginTime(void);
  void determineLoginMode(void);
};

//-
//  The generated code includes comments that allow you to trace directly
//  back to the appropriate location in the model.  The basic format
//  is <system>/block_name, where system is the system number (uniquely
//  assigned by Simulink) and block_name is the name of the block.
//
//  Note that this particular code originates from a subsystem build,
//  and has its own system numbers different from the parent model.
//  Refer to the system hierarchy for this subsystem below, and use the
//  MATLAB hilite_system command to trace the generated code back
//  to the parent model.  For example,
//
//  hilite_system('login_control/MarketLoginControl')    - opens subsystem login_control/MarketLoginControl
//  hilite_system('login_control/MarketLoginControl/Kp') - opens and selects block Kp
//
//  Here is the system hierarchy for this model
//
//  '<Root>' : 'login_control'
//  '<S1>'   : 'login_control/MarketLoginControl'

#endif                                 // RTW_HEADER_MarketLoginControl_h_

//
// File trailer for generated code.
//
// [EOF]
//
