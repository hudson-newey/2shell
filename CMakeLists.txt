cmake_minimum_required(VERSION 3.10)
project(2sh C)

# I really don't ever want to see a slow version of 2sh
# so cmake should build in release mode by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(2sh src/main.c)
target_link_libraries(2sh readline)
