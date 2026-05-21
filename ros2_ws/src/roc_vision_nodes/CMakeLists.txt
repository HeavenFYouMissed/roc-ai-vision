cmake_minimum_required(VERSION 3.20)
project(roc_vision_nodes)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

# Node executables will be added during Phase 4 implementation.

ament_package()
