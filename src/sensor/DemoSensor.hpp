#ifndef DEMO_SENSOR_HPP
#define DEMO_SENSOR_HPP

#include <optional>

#include "sensor/ISensor.hpp"

namespace sensor {

class DemoSensor : public ISensor {
public:
  std::optional<state::SensorSnapshot> read(config::Room room) override;
};

} // namespace sensor

#endif // DEMO_SENSOR_HPP
