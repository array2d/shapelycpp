cmake_minimum_required(VERSION 3.16)
project(shapelycpp_example LANGUAGES CXX)

find_package(GEOS REQUIRED)

add_executable(example main.cpp)
target_include_directories(example PRIVATE ${CMAKE_SOURCE_DIR}/..)
target_link_libraries(example PRIVATE GEOS::geos)
set_target_properties(example PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
