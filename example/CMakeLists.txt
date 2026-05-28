cmake_minimum_required(VERSION 3.16)
project(shapelycpp_example LANGUAGES CXX)

find_package(shapelycpp REQUIRED)
find_package(GEOS REQUIRED)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE shapelycpp::shapelycpp GEOS::geos_c)
