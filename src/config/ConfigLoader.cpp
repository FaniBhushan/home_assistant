#include "config/ConfigLoader.hpp"
#include "logger/logger.hpp"

#include <cctype>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace config {

using json = nlohmann::json;

namespace {

template <typename T, typename Validator>
void assignIfValid(const json& configJson, const char* key, T& target,
                   Validator&& validator) {
  const auto it = configJson.find(key);
  if (it == configJson.end()) {
    return;
  }

  try {
    const T candidate = it->get<T>();
    if (validator(candidate)) {
      target = candidate;
    } else {
      logger::warning("ConfigLoader: invalid value for key '" +
                      std::string(key) + "', using default");
    }
  } catch (const json::exception&) {
    logger::warning("ConfigLoader: invalid type for key '" +
                    std::string(key) + "', using default");
  }
}

std::string normalizeRoomName(std::string value) {
  for (char& ch : value) {
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    if (ch == '-' || ch == '_') {
      ch = ' ';
    }
  }
  return value;
}

Room parseRoomEnum(const std::string& value) {
  const std::string normalized = normalizeRoomName(value);
  if (normalized == "living room") {
    return Room::livingRoom;
  }
  if (normalized == "bedroom") {
    return Room::bedroom;
  }
  if (normalized == "kitchen") {
    return Room::kitchen;
  }
  if (normalized == "bathroom") {
    return Room::bathroom;
  }
  if (normalized == "office") {
    return Room::office;
  }
  if (normalized == "dining room") {
    return Room::diningRoom;
  }
  return Room::unknown;
}

} // namespace

ConfigLoader::ConfigLoader(const std::string& configPath) {
  std::ifstream ifs(configPath);
  if (!ifs.is_open()) {
    logger::warning("ConfigLoader: failed to open config file, using defaults: " +
                    configPath);
    return;
  }

  try {
    json configJson = json::parse(ifs);

    assignIfValid<int>(configJson, "port", m_port,
                       [](const int value) { return value > 0; });
    assignIfValid<int>(configJson, "sensor_interval_seconds",
                       m_sensorIntervalSeconds,
                       [](const int value) { return value > 0; });

    if (configJson.contains("rooms") && configJson["rooms"].is_array()) {
      std::vector<std::string> parsedRooms;
      std::vector<Room> parsedRoomEnums;
      for (const auto& room : configJson["rooms"]) {
        if (room.is_string()) {
          const std::string roomName = room.get<std::string>();
          parsedRooms.push_back(roomName);

          const Room roomEnum = parseRoomEnum(roomName);
          if (roomEnum == Room::unknown) {
            logger::warning("ConfigLoader: unknown room type '" + roomName +
                            "', mapped to unknown");
          }
          parsedRoomEnums.push_back(roomEnum);
        } else {
          logger::warning("ConfigLoader: non-string room entry ignored");
        }
      }
      if (!parsedRooms.empty()) {
        m_rooms = std::move(parsedRooms);
        m_roomEnums = std::move(parsedRoomEnums);
      }
    }

    logger::info("ConfigLoader: loaded config from " + configPath);
  } catch (const json::exception& ex) {
    logger::warning(
        "ConfigLoader: invalid config JSON, using defaults: " +
        std::string(ex.what()));
  }
}

int ConfigLoader::getPort() const { return m_port; }

int ConfigLoader::getSensorIntervalSeconds() const {
  return m_sensorIntervalSeconds;
}

const std::vector<std::string>& ConfigLoader::getRooms() const {
  return m_rooms;
}

const std::vector<Room>& ConfigLoader::getRoomEnums() const {
  return m_roomEnums;
}

} // namespace config
