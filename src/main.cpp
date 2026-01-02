#include <chrono>
#include <thread>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include "logger/logger.hpp"

int main() {
  logger::init();

  logger::info("Home Assistant started");

  while (true) {
    logger::info("Home Assistant running");
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  return 0;
}