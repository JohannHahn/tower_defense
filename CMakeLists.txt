cmake_minimum_required(VERSION 3.15)

project(tower_defense)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(raylib)

add_executable(tower_defense main.cpp)

target_include_directories(tower_defense PRIVATE raylib/src)

