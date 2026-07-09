#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

#include <string>
#include <string_view>
#include <vector>


namespace config {

enum class Room {
  livingRoom,
  bedroom,
  kitchen,
  bathroom,
  office,
  diningRoom,
  unknown,
};

constexpr std::string_view to_string(const Room room) {
  switch (room) {
  case Room::livingRoom:
    return "livingRoom";
  case Room::bedroom:
    return "bedroom";
  case Room::kitchen:
    return "kitchen";
  case Room::bathroom:
    return "bathroom";
  case Room::office:
    return "office";
  case Room::diningRoom:
    return "diningRoom";
  case Room::unknown:
  default:
    return "unknown";
  }
}

class ConfigLoader {
public:
  explicit ConfigLoader(const std::string& configPath);

  int getPort() const;
  int getSensorIntervalSeconds() const;
  const std::vector<std::string>& getRooms() const;
  const std::vector<Room>& getRoomEnums() const;

private:
  int m_port{8080};
  int m_sensorIntervalSeconds{5};
  std::vector<std::string> m_rooms;
  std::vector<Room> m_roomEnums;
};

} // namespace config
#endif // CONFIG_LOADER_HPP
