#include <gtest/gtest.h>

#include "sensor/DemoSensor.hpp"

TEST(DemoSensorTest, ReturnsReadingForKnownRoom) {
  sensor::DemoSensor sensor;

  const auto known = sensor.read(config::Room::livingRoom);

  ASSERT_TRUE(known.has_value());
  EXPECT_FLOAT_EQ(known->humidity, 45.0f);
}

TEST(DemoSensorTest, ReturnsNulloptForUnknownRoom) {
  sensor::DemoSensor sensor;

  const auto unknown = sensor.read(config::Room::unknown);

  EXPECT_FALSE(unknown.has_value());
}
