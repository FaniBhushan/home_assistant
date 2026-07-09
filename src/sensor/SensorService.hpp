#ifndef SENSOR_SERVICE_HPP
#define SENSOR_SERVICE_HPP

#include <boost/asio.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "config/ConfigLoader.hpp"
#include "sensor/ISensor.hpp"
#include "state/StateRegistry.hpp"

namespace sensor {

class SensorService {
public:
  using SensorCallback =
      std::function<void(config::Room room, const state::SensorSnapshot&)>;

  SensorService(boost::asio::io_context& ioContext,
                std::shared_ptr<config::ConfigLoader> config,
                std::shared_ptr<ISensor> sensor);

  std::size_t subscribe(SensorCallback callback);
  void unsubscribe(std::size_t subscriberId);

  void start();
  void stop();

private:
  void scheduleNextTick();
  void pollAllRooms();
  void notifySubscribers(config::Room room,
                         const state::SensorSnapshot& snapshot);

  boost::asio::io_context& m_ioContext;
  boost::asio::steady_timer m_timer;
  std::shared_ptr<config::ConfigLoader> m_config;
  std::shared_ptr<ISensor> m_sensor;

  std::mutex m_subscriberMutex;
  std::unordered_map<std::size_t, SensorCallback> m_subscribers;
  std::size_t m_nextSubscriberId{1};
  bool m_running{false};
};

} // namespace sensor

#endif // SENSOR_SERVICE_HPP