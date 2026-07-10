#ifndef TEST_SUPPORT_HPP
#define TEST_SUPPORT_HPP

#include <atomic>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#include "sensor/ISensor.hpp"

namespace test_support {

inline std::string writeTempConfigWithContent(const std::string& content) {
  const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  std::ostringstream path;
  path << "/tmp/home_assistant_test_config_" << now << "_custom.json";

  std::ofstream ofs(path.str());
  ofs << content;
  ofs.close();

  return path.str();
}

inline std::string writeDefaultTempConfig() {
  return writeTempConfigWithContent(
      "{\n"
      "  \"sensor_interval_seconds\": 1,\n"
      "  \"rooms\": [\"living room\"]\n"
      "}\n");
}

inline void removeFile(const std::string& path) { std::remove(path.c_str()); }

class FakeSensor : public sensor::ISensor {
public:
  explicit FakeSensor(state::SensorSnapshot snapshot) : m_snapshot(snapshot) {}

  std::optional<state::SensorSnapshot> read(config::Room room) override {
    ++m_reads;
    if (room == config::Room::unknown) {
      return std::nullopt;
    }
    return m_snapshot;
  }

  int reads() const { return m_reads.load(); }

private:
  state::SensorSnapshot m_snapshot;
  std::atomic<int> m_reads{0};
};

} // namespace test_support

#endif // TEST_SUPPORT_HPP
