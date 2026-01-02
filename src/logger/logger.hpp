#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logger {
namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;

void init() {
  logging::add_file_log(
      keywords::file_name = "/home/bhufani/home_assistant/logs/core.log",
      keywords::auto_flush = true, keywords::rotation_size = 10 * 1024 * 1024,
      keywords::max_size = 100 * 1024 * 1024, keywords::max_files = 10,
      keywords::time_based_rotation =
          sinks::file::rotation_at_time_point(0, 0, 0),
      keywords::format =
          (expr::stream << expr::format_date_time<boost::posix_time::ptime>(
                               "TimeStamp", "%Y-%m-%d %H:%M:%S,%f")
                        << " - " << logging::trivial::severity
                        << " - [home_assistant] - " << expr::smessage));

  logging::add_common_attributes();
}

void info(const std::string &msg) {
  logging::sources::severity_logger_mt<logging::trivial::severity_level> lg;
  BOOST_LOG_SEV(lg, logging::trivial::info) << msg;
}

void error(const std::string &msg) {
  logging::sources::severity_logger_mt<logging::trivial::severity_level> lg;
  BOOST_LOG_SEV(lg, logging::trivial::error) << msg;
}

void debug(const std::string &msg) {
  logging::sources::severity_logger_mt<logging::trivial::severity_level> lg;
  BOOST_LOG_SEV(lg, logging::trivial::debug) << msg;
}

void warning(const std::string &msg) {
  logging::sources::severity_logger_mt<logging::trivial::severity_level> lg;
  BOOST_LOG_SEV(lg, logging::trivial::warning) << msg;
}

void trace(const std::string &msg) {
  logging::sources::severity_logger_mt<logging::trivial::severity_level> lg;
  BOOST_LOG_SEV(lg, logging::trivial::trace) << msg;
}

} // namespace Logger
#endif // LOGGER_HPP
