cmake_minimum_required(VERSION 3.12)

project(tower_defense)
add_subdirectory(raylib)
include_directories(includes)

add_executable(tower_defense main.cpp)
set_property(TARGET tower_defense PROPERTY CXX_STANDARD 23)

target_link_libraries(tower_defense raylib)
