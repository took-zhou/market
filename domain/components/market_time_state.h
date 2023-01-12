#ifndef RTW_HEADER_MarketLoginState_h_
#define RTW_HEADER_MarketLoginState_h_
#include <stdint.h>

enum TimeState { kReserve = 0, kLoginTime = 1, kLogoutTime = 2 };

enum SubTimeState {
  kInDayPrePare = 1,
  kInDayLogin = 2,
  kInDayLogout = 3,
  kInInitSts = 4,
  kInNightPrePare = 5,
  kInNightLogin = 6,
  kInNightLogout = 7
};

struct MarketTimeState {
 public:
  void Step();
  void Update(void);

  MarketTimeState();
  ~MarketTimeState();

  void SetTimeState(int command);
  void SetSubTimeState(int command);
  TimeState GetTimeState();
  SubTimeState GetSubTimeState();
  struct tm *GetTimeNow();

 private:
  uint8_t is_active_step_ = false;
  SubTimeState sub_time_state_ = kInInitSts;
  SubTimeState debug_sub_time_state_ = kInInitSts;
  TimeState time_state_ = kReserve;
  TimeState debug_time_state_ = kReserve;
  struct tm *timenow_ = nullptr;

  int32_t now_mins_;
  int32_t day_prepare_mins_;
  int32_t day_login_mins_;
  int32_t day_logout_mins_;
  int32_t night_prepare_mins_;
  int32_t night_login_mins_;
  int32_t night_logout_mins_;

  uint8_t IsDuringDayLogoutTime(void);
  uint8_t IsDuringNightLogoutTime(void);
  uint8_t IsDuringDayLoginTime(void);
  uint8_t IsDuringNightLoginTime(void);
  uint8_t IsDuringDayPrepareTime(void);
  uint8_t IsDuringNightPrepareTime(void);
};

#endif
