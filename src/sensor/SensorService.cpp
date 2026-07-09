#include "sensor/SensorService.hpp"

#include "logger/logger.hpp"

#include <algorithm>
#include <chrono>
#include <utility>
#include <vector>

namespace sensor {

SensorService::SensorService(boost::asio::io_context& ioContext,
                             std::shared_ptr<config::ConfigLoader> config,
                             std::shared_ptr<ISensor> sensor)
    : m_ioContext(ioContext),
      m_timer(m_ioContext),
      m_config(std::move(config)),
      m_sensor(std::move(sensor)) {}

std::size_t SensorService::subscribe(SensorCallback callback) {
  std::lock_guard<std::mutex> lock(m_subscriberMutex);
  const std::size_t id = m_nextSubscriberId++;
  m_subscribers.emplace(id, std::move(callback));
  return id;
}

void SensorService::unsubscribe(std::size_t subscriberId) {
  std::lock_guard<std::mutex> lock(m_subscriberMutex);
  m_subscribers.erase(subscriberId);
}

void SensorService::start() {
  if (m_running) {
    return;
  }

  m_running = true;
  scheduleNextTick();
}

void SensorService::stop() {
  m_running = false;
  boost::system::error_code ec;
  m_timer.cancel(ec);
}

void SensorService::scheduleNextTick() {
  if (!m_running) {
    return;
  }

  const int intervalSeconds =
      std::max(1, m_config ? m_config->getSensorIntervalSeconds() : 1);
  m_timer.expires_after(std::chrono::seconds(intervalSeconds));
  m_timer.async_wait([this](const boost::system::error_code& ec) {
    if (ec || !m_running) {
      return;
    }

    pollAllRooms();
    scheduleNextTick();
  });
}

void SensorService::pollAllRooms() {
  if (!m_sensor || !m_config) {
    return;
  }

  for (const auto room : m_config->getRoomEnums()) {
    const auto reading = m_sensor->read(room);
    if (!reading.has_value()) {
      logger::warning("SensorService: failed to read sensor for room=" +
                      std::string(config::to_string(room)));
      continue;
    }
    notifySubscribers(room, reading.value());
  }
}

void SensorService::notifySubscribers(config::Room room,
                                      const state::SensorSnapshot& snapshot) {
  std::vector<SensorCallback> callbacks;
  {
    std::lock_guard<std::mutex> lock(m_subscriberMutex);
    callbacks.reserve(m_subscribers.size());
    for (const auto& entry : m_subscribers) {
      callbacks.push_back(entry.second);
    }
  }

  for (const auto& callback : callbacks) {
    callback(room, snapshot);
  }
}

} // namespace sensor