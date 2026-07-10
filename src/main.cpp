#include <chrono>
#include <csignal>
#include <condition_variable>
#include <memory>
#include <mutex>

#include <boost/asio/signal_set.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

#include "config/ConfigLoader.hpp"
#include "logger/logger.hpp"
#include "sensor/DemoSensor.hpp"
#include "sensor/SensorService.hpp"
#include "service/HAService.hpp"
#include "state/StateRegistry.hpp"

int main() {
  logger::init();

  logger::info("Home Assistant started");

  auto configLoader =
      std::make_shared<config::ConfigLoader>("config/home_assistant.json");

  service::HAService haService;
  haService.init();

  auto sensorImpl = std::make_shared<sensor::DemoSensor>();
  sensor::SensorService sensorService(haService.ioContext(), configLoader,
                                      sensorImpl);

  auto stateRegistry = std::make_shared<state::StateRegistry>(configLoader);
  stateRegistry->registerWithSensorService(sensorService);

  std::mutex shutdownMutex;
  std::condition_variable shutdownCv;
  bool shuttingDown = false;

  boost::asio::signal_set signals(haService.ioContext(), SIGINT, SIGTERM);
  signals.async_wait(
      [&](const boost::system::error_code& ec, int signalNumber) {
        if (ec) {
          return;
        }

        logger::info("Shutdown signal received: " +
                     std::to_string(signalNumber));
        sensorService.stop();
        stateRegistry->unregisterFromSensorService();

        {
          std::lock_guard<std::mutex> lock(shutdownMutex);
          shuttingDown = true;
        }
        shutdownCv.notify_one();
      });

  sensorService.start();
  logger::info("Home Assistant components wired and running");

  {
    std::unique_lock<std::mutex> lock(shutdownMutex);
    shutdownCv.wait(lock, [&]() { return shuttingDown; });
  }

  haService.stop();
  logger::info("Home Assistant stopped");

  return 0;
}