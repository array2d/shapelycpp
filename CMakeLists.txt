cmake_minimum_required(VERSION 3.16)

# -- Auto-detect version from pip's shapely -----------------------------------
execute_process(
    COMMAND python3 -c "import shapely; print(shapely.__version__)"
    OUTPUT_VARIABLE SHAPELY_PIP_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE _pip_rc
)
if(NOT _pip_rc EQUAL 0 OR SHAPELY_PIP_VERSION STREQUAL "")
    message(FATAL_ERROR "Cannot detect shapely version from pip. Install shapely first: pip install shapely")
endif()

project(shapelycpp VERSION ${SHAPELY_PIP_VERSION} LANGUAGES CXX)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- INTERFACE target for find_package consumers ----------------------------
add_library(shapelycpp INTERFACE)
target_include_directories(shapelycpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/shapelycpp>
)
target_compile_features(shapelycpp INTERFACE cxx_std_17)

# ---- Install — header-only C++ library --------------------------------------
# shapely/     — pure native C++ (zero pybind11 dependency)
install(DIRECTORY shapely
    DESTINATION include/shapelycpp
    FILES_MATCHING PATTERN "*.h"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    shapelycpp-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/shapelycpp-config.cmake
    INSTALL_DESTINATION lib/cmake/shapelycpp
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/shapelycpp-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/shapelycpp-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/shapelycpp-config-version.cmake
    DESTINATION lib/cmake/shapelycpp
)

# ---- Docs & examples -------------------------------------------------------
install(FILES README.md
    DESTINATION share/shapelycpp
)
install(DIRECTORY example/
    DESTINATION share/shapelycpp/example
)

# ---- CPack DEB packaging ----------------------------------------------------
set(CPACK_PACKAGE_NAME                     "shapelycpp-dev")
set(CPACK_PACKAGE_VERSION                  ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR                   "array2d")
set(CPACK_PACKAGE_CONTACT                  "array2d")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY      "C++ pixel-level alignment of Python shapely — header-only library")
set(CPACK_GENERATOR                        "DEB")
set(CPACK_DEB_COMPONENT_INSTALL            OFF)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER        "array2d")
set(CPACK_DEBIAN_PACKAGE_SECTION           "libdevel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY          "optional")
set(CPACK_DEBIAN_PACKAGE_DEPENDS           "libgeos-dev")
set(CPACK_DEBIAN_FILE_NAME                 "shapelycpp-dev-${CPACK_PACKAGE_VERSION}-Linux.deb")
set(CPACK_PACKAGING_INSTALL_PREFIX         "/usr")

include(CPack)

# Convenience target
add_custom_target(deb
    COMMAND cpack -G DEB
    COMMENT "Packaging shapelycpp-dev-${CPACK_PACKAGE_VERSION}-Linux.deb"
)

# ---- Tests (optional, requires pybind11 + GEOS) ------------------------------
option(BUILD_TESTS "Build pybind11 test module" OFF)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

message(STATUS "shapelycpp v${PROJECT_VERSION}  (header-only C++ library)")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  DEB:  make deb  →  shapelycpp-dev-${CPACK_PACKAGE_VERSION}-Linux.deb")
message(STATUS "  Install: sudo apt-get install -y libgeos-dev")
