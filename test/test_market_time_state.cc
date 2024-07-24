#include <memory>
#include "common/self/file_util.h"
#include "gtest/gtest.h"
#include "market/domain/components/market_time_state.h"

namespace {
struct MarketTimeStateTest : public testing::Test {
 public:
  void SetUp() override {
    auto &json_cfg = utils::JsonConfig::GetInstance();
    json_cfg.SetFileName("/etc/marktrade/config.json");
    json_cfg.WriteConfig("market", "LogInTimeList", "08:06-02:15");
    market_time_state_ = std::make_unique<MarketTimeState>();
  }

  void TearDown() override {}
  std::unique_ptr<MarketTimeState> &GetMarketTimeState() { return market_time_state_; };
  void PrintStatus() {
    printf("sub time state: %d time state: %d.\r\n", market_time_state_->GetSubTimeState(), market_time_state_->GetTimeState());
  }

 private:
  std::unique_ptr<MarketTimeState> market_time_state_;
};

TEST_F(MarketTimeStateTest, state_change) {
  GetMarketTimeState()->Simulate("2024-07-23 07:00:00");
  PrintStatus();
  GetMarketTimeState()->Simulate("2024-07-23 07:01:00");
  PrintStatus();
  GetMarketTimeState()->Simulate("2024-07-23 09:00:00");
  PrintStatus();
  GetMarketTimeState()->Simulate("2024-07-23 09:01:00");
  PrintStatus();
  GetMarketTimeState()->Simulate("2024-07-23 16:00:00");
  PrintStatus();
  GetMarketTimeState()->Simulate("2024-07-23 16:01:00");
  PrintStatus();
  GetMarketTimeState()->Simulate("2024-07-23 21:00:00");
  PrintStatus();
  GetMarketTimeState()->Simulate("2024-07-23 21:01:00");
  PrintStatus();
  GetMarketTimeState()->Simulate("2024-07-24 03:00:00");
  PrintStatus();
  GetMarketTimeState()->Simulate("2024-07-24 03:01:00");
  PrintStatus();
};
}  // namespace