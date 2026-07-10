cmake_minimum_required(VERSION 3.20)
project(home_assistant)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(patches)

find_package(Boost 1.74 REQUIRED COMPONENTS log)
include(FetchContent)

add_library(home_assistant_core
  src/config/ConfigLoader.cpp
  src/sensor/DemoSensor.cpp
  src/sensor/SensorService.cpp
  src/state/StateRegistry.cpp
  src/service/HAService.cpp
)

target_include_directories(home_assistant_core PUBLIC src)

target_link_libraries(home_assistant_core PUBLIC Boost::log)

add_executable(home_assistant
  src/main.cpp
)

target_include_directories(home_assistant PRIVATE src)

target_link_libraries(home_assistant PRIVATE home_assistant_core)

enable_testing()

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(home_assistant_tests
  src/sensor/tests/DemoSensorTest.cpp
  src/sensor/tests/SensorServiceTest.cpp
  src/service/tests/HAServiceTest.cpp
  src/state/tests/StateRegistryTest.cpp
)

target_include_directories(home_assistant_tests PRIVATE src)
target_link_libraries(home_assistant_tests PRIVATE home_assistant_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(home_assistant_tests)
