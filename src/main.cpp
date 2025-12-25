#include <chrono>
#include <thread>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

void init_logging() {
  logging::add_file_log(
      keywords::file_name = "/home/bhufani/home_assistant/logs/core.log", keywords::auto_flush = true,
      keywords::format =
          (expr::stream << expr::format_date_time<boost::posix_time::ptime>(
                               "TimeStamp", "%Y-%m-%d %H:%M:%S,%f")
                        << " - " << logging::trivial::severity
                        << " - [home_assistant] - " << expr::smessage));

  logging::add_common_attributes();
}

int main() {
  init_logging();

  BOOST_LOG_TRIVIAL(info) << "Home Assistant started";

  while (true) {
    BOOST_LOG_TRIVIAL(info) << "Home Assistant running";
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  return 0;
}