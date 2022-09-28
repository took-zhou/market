#ifndef RTW_HEADER_MarketLoginState_h_
#define RTW_HEADER_MarketLoginState_h_
#include <stdint.h>

enum TimeState { kReserve = 0, kLoginTime = 1, kLogoutTime = 2 };

enum SubTimeState { kInDayLogin = 1, kInDayLogout = 2, kInInitSts = 3, kInNightLogin = 4, kInNightLogout = 5 };

struct MarketTimeState {
 public:
  void Step();
  void Update(void);

  MarketTimeState();
  ~MarketTimeState();

  void SetTimeState(int command);
  TimeState GetTimeState();
  SubTimeState GetSubTimeState();

 private:
  uint8_t is_active_step_ = false;
  SubTimeState sub_time_state_;
  TimeState time_state_ = kReserve;
  TimeState debug_time_state_ = kReserve;

  uint32_t now_mins_;
  uint32_t day_login_mins_;
  uint32_t day_logout_mins_;
  uint32_t night_login_mins_;
  uint32_t night_logout_mins_;

  uint8_t IsDuringDayLogoutTime(void);
  uint8_t IsDuringNightLogoutTime(void);
  uint8_t IsDuringDayLoginTime(void);
  uint8_t IsDuringNightLoginTime(void);
};

#endif
