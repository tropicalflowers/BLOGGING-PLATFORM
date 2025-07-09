cmake_minimum_required(VERSION 3.10)
project(BloggingPlatform)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

include_directories("${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/include")

add_executable(blog main.cpp)
