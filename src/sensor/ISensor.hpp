#ifndef I_SENSOR_HPP
#define I_SENSOR_HPP

#include <optional>

#include "config/ConfigLoader.hpp"
#include "state/StateRegistry.hpp"

namespace sensor {

class ISensor {
public:
    virtual ~ISensor() = default;
    virtual std::optional<state::SensorSnapshot> read(config::Room room) = 0;
};

} // namespace sensor

#endif // I_SENSOR_HPP