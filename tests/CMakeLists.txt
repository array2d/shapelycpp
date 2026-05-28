# ---- Dependencies -----------------------------------------------------------
find_package(pybind11 REQUIRED)
find_package(GEOS REQUIRED)

# ---- Build test module (named shapelycpp_test to avoid clash with header-only target) ----
pybind11_add_module(shapelycpp_test module.cpp)
target_include_directories(shapelycpp_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(shapelycpp_test PRIVATE GEOS::geos)

# Rename the output .so to "shapelycpp" so Python "import shapelycpp" works
set_target_properties(shapelycpp_test PROPERTIES OUTPUT_NAME shapelycpp)

# ---- Test target: run from repo root so "tests/" is a package ---------------
add_test(
    NAME shapelycpp_all
    COMMAND python3 -m pytest tests/ -v
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set_tests_properties(shapelycpp_all
    PROPERTIES ENVIRONMENT
        "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}:${CMAKE_SOURCE_DIR}:$ENV{PYTHONPATH}"
)
