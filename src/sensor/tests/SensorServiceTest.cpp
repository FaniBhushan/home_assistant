#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "config/ConfigLoader.hpp"
#include "sensor/SensorService.hpp"
#include "service/HAService.hpp"
#include "test_support/TestSupport.hpp"

TEST(SensorServiceTest, UnsubscribeStopsNotifications) {
  const std::string configPath = test_support::writeDefaultTempConfig();
  auto configLoader = std::make_shared<config::ConfigLoader>(configPath);

  service::HAService haService;
  haService.init();

  auto sensorImpl = std::make_shared<test_support::FakeSensor>(
      state::SensorSnapshot{23.0f, 650.0f, 42.0f});
  sensor::SensorService sensorService(haService.ioContext(), configLoader,
                                      sensorImpl);

  std::atomic<int> callbackCount{0};
  const auto subscriberId = sensorService.subscribe(
      [&](config::Room, const state::SensorSnapshot&) { ++callbackCount; });

  sensorService.unsubscribe(subscriberId);
  sensorService.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(1300));

  sensorService.stop();
  haService.stop();
  test_support::removeFile(configPath);

  EXPECT_EQ(callbackCount.load(), 0);
}

TEST(SensorServiceTest, NotifiesSubscribedCallbacks) {
  const std::string configPath = test_support::writeDefaultTempConfig();
  auto configLoader = std::make_shared<config::ConfigLoader>(configPath);

  service::HAService haService;
  haService.init();

  auto sensorImpl = std::make_shared<test_support::FakeSensor>(
      state::SensorSnapshot{24.5f, 742.0f, 49.0f});
  sensor::SensorService sensorService(haService.ioContext(), configLoader,
                                      sensorImpl);

  std::atomic<int> callbackCount{0};
  const auto subscriberId = sensorService.subscribe(
      [&](config::Room, const state::SensorSnapshot&) { ++callbackCount; });

  sensorService.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(1300));

  sensorService.stop();
  sensorService.unsubscribe(subscriberId);
  haService.stop();
  test_support::removeFile(configPath);

  EXPECT_GT(callbackCount.load(), 0);
  EXPECT_GT(sensorImpl->reads(), 0);
}
