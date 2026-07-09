#include "state/StateRegistry.hpp"

#include "logger/logger.hpp"
#include "sensor/SensorService.hpp"

#include <string>
#include <utility>

namespace state {

StateRegistry::~StateRegistry() { unregisterFromSensorService(); }

StateRegistry::StateRegistry(std::shared_ptr<config::ConfigLoader> config)
    : m_config(std::move(config)) {}

void StateRegistry::registerWithSensorService(
    sensor::SensorService& sensorService) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_subscriptionId.has_value()) {
    return;
  }

  const std::weak_ptr<StateRegistry> weakSelf = weak_from_this();
  if (weakSelf.expired()) {
    logger::error(
        "StateRegistry: must be owned by std::shared_ptr before registering "
        "with SensorService");
    return;
  }

  m_subscriptionId =
      sensorService.subscribe([weakSelf](config::Room room,
                                         const SensorSnapshot& snapshot) {
        if (const auto self = weakSelf.lock()) {
          self->update(room, snapshot);
        }
      });
  m_sensorService = &sensorService;
  logger::info("StateRegistry: registered with SensorService");
}

void StateRegistry::unregisterFromSensorService() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_sensorService || !m_subscriptionId.has_value()) {
    return;
  }

  m_sensorService->unsubscribe(*m_subscriptionId);
  m_subscriptionId.reset();
  m_sensorService = nullptr;
  logger::info("StateRegistry: unregistered from SensorService");
}

void StateRegistry::update(const config::Room& room,
                           const SensorSnapshot& snapshot) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_roomSensorData[room] = snapshot;
  logger::debug("StateRegistry: updated room=" +
                std::string(config::to_string(room)) +
                " with temperature=" + std::to_string(snapshot.temperature) +
                ", co2=" + std::to_string(snapshot.co2) +
                ", humidity=" + std::to_string(snapshot.humidity));
}

std::optional<SensorSnapshot> StateRegistry::get(
    const config::Room room) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_roomSensorData.find(room);
  if (it == m_roomSensorData.end()) {
    return std::nullopt;
  }
  return it->second;
}

} // namespace state

