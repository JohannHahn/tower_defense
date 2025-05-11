cmake_minimum_required(VERSION 3.15)

project(tower_defense)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_subdirectory(raylib)

add_executable(tower_defense main.cpp)

target_link_libraries(tower_defense raylib)

target_include_directories(tower_defense PRIVATE raylib/raylib/include/)

