cmake_minimum_required(VERSION 3.20)
project(home_assistant)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(patches)

find_package(Boost 1.74 CONFIG REQUIRED COMPONENTS log)

add_executable(home_assistant src/main.cpp)
target_link_libraries(home_assistant PRIVATE Boost::log)