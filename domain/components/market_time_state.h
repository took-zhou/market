#ifndef RTW_HEADER_MarketLoginState_h_
#define RTW_HEADER_MarketLoginState_h_
#include <stdint.h>

enum TimeState { kReserve = 0, kLoginTime = 1, kLogoutTime = 2 };

enum SubTimeState { kInDayLogin = 1, kInDayLogout = 2, kInInitSts = 3, kInNightLogin = 4, kInNightLogout = 5 };

class MarketTimeState {
 public:
  void Step();
  void Update(void);

  MarketTimeState();
  ~MarketTimeState();

  void set_time_state(int command);
  TimeState get_time_state();
  SubTimeState get_sub_time_state();

 private:
  uint8_t is_active_step = false;
  SubTimeState sub_time_state;
  TimeState time_state = kReserve;
  TimeState debug_time_state = kReserve;

  uint8_t now_mins;
  uint8_t day_login_mins;
  uint8_t day_logout_mins;
  uint8_t night_login_mins;
  uint8_t night_logout_mins;

  uint8_t IsDuringDayLogoutTime(void);
  uint8_t IsDuringNightLogoutTime(void);
  uint8_t IsDuringDayLoginTime(void);
  uint8_t IsDuringNightLoginTime(void);
};

#endif
