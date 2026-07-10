#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <optional>
#include <thread>

#include "config/ConfigLoader.hpp"
#include "sensor/SensorService.hpp"
#include "service/HAService.hpp"
#include "state/StateRegistry.hpp"
#include "test_support/TestSupport.hpp"

TEST(StateRegistryTest, UpdateAndGetStoresSnapshot) {
  const std::string configPath = test_support::writeDefaultTempConfig();
  auto configLoader = std::make_shared<config::ConfigLoader>(configPath);
  auto stateRegistry = std::make_shared<state::StateRegistry>(configLoader);

  const state::SensorSnapshot snapshot{21.2f, 700.0f, 41.5f};
  stateRegistry->update(config::Room::office, snapshot);

  const auto stored = stateRegistry->get(config::Room::office);
  ASSERT_TRUE(stored.has_value());
  EXPECT_FLOAT_EQ(stored->temperature, snapshot.temperature);
  EXPECT_FLOAT_EQ(stored->co2, snapshot.co2);
  EXPECT_FLOAT_EQ(stored->humidity, snapshot.humidity);

  test_support::removeFile(configPath);
}

TEST(StateRegistryTest, ReceivesUpdatesFromSensorService) {
  const std::string configPath = test_support::writeDefaultTempConfig();
  auto configLoader = std::make_shared<config::ConfigLoader>(configPath);

  service::HAService haService;
  haService.init();

  auto sensorImpl = std::make_shared<test_support::FakeSensor>(
      state::SensorSnapshot{24.5f, 742.0f, 49.0f});
  sensor::SensorService sensorService(haService.ioContext(), configLoader,
                                      sensorImpl);

  auto stateRegistry = std::make_shared<state::StateRegistry>(configLoader);
  stateRegistry->registerWithSensorService(sensorService);
  sensorService.start();

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  std::optional<state::SensorSnapshot> latest;
  while (std::chrono::steady_clock::now() < deadline) {
    latest = stateRegistry->get(config::Room::livingRoom);
    if (latest.has_value()) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
  }

  sensorService.stop();
  stateRegistry->unregisterFromSensorService();
  haService.stop();
  test_support::removeFile(configPath);

  EXPECT_TRUE(latest.has_value());
}
