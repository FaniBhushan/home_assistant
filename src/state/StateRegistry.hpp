#ifndef STATE_REGISTRY_HPP
#define STATE_REGISTRY_HPP

#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

#include "config/ConfigLoader.hpp"

namespace sensor {
class SensorService;
}

namespace state {

struct SensorSnapshot {
  float temperature;
  float co2;
  float humidity;
};

class StateRegistry : public std::enable_shared_from_this<StateRegistry> {
public:
  ~StateRegistry();
  StateRegistry(std::shared_ptr<config::ConfigLoader> config);

  void registerWithSensorService(sensor::SensorService& sensorService);
  void unregisterFromSensorService();

  void update(const config::Room& room, const SensorSnapshot& snapshot);
  std::optional<SensorSnapshot> get(const config::Room& room) const;

private:
  std::shared_ptr<config::ConfigLoader> m_config;
  mutable std::mutex m_mutex;
  std::unordered_map<config::Room, SensorSnapshot> m_roomSensorData;
  sensor::SensorService* m_sensorService{nullptr};
  std::optional<std::size_t> m_subscriptionId;
};

} // namespace state
#endif // STATE_REGISTRY_HPP
