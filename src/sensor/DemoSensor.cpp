#include "sensor/DemoSensor.hpp"

#include <chrono>

namespace sensor {

std::optional<state::SensorSnapshot> DemoSensor::read(config::Room room) {
  if (room == config::Room::unknown) {
    return std::nullopt;
  }

  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  const auto seconds =
      static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(now)
                             .count() %
                         60);

  state::SensorSnapshot snapshot{};
  snapshot.temperature = 22.0f + (seconds * 0.01f);
  snapshot.co2 = 600.0f + (seconds * 2.0f);
  snapshot.humidity = 45.0f;
  return snapshot;
}

} // namespace sensor
