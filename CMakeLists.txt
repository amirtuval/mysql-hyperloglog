cmake_minimum_required(VERSION 2.8)
project(MYSQLHLL128)

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_subdirectory (libmysqlhll128)

