# shapelycpp

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-%3E%3D3.16-green.svg)](https://cmake.org/)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

## Background

Shapely is the de-facto standard for planar geometry operations in Python — but its ceiling is locked by Python.

We created `shapelycpp` to keep Shapely's familiar API while letting C++ break through Python's performance ceiling and accelerate your code further.

## Overview

`shapelycpp` is a **header-only C++ library** implementing shapely's core geometry API (`shapely.geometry.*`, `shapely.ops.*`) with pixel-level precision alignment. Template-based design supports both `float` and `double` coordinates. Powered by GEOS for robust computational geometry.

## Quick Start

### Dependencies

- **C++17** compiler (GCC >= 9, Clang >= 7, MSVC >= 2019)
- **GEOS** >= 3.9 (libgeos-dev on Ubuntu)
- **pybind11** (optional, for Python bindings)

### Usage

```cpp
#include "shapely/geometry/point.h"
#include "shapely/geometry/linestring.h"

// Create a point
shapely::geometry::Point<double> p1(0.0, 0.0);
shapely::geometry::Point<double> p2(3.0, 4.0);

double d = p1.distance(p2);  // → 5.0

// Create a polygon with buffer
auto buffered = p1.buffer(10.0);
```

### Install

**Ubuntu (DEB)**

```bash
sudo apt-get install -y libgeos-dev
```

Download the [latest `.deb` release](https://github.com/array2d/shapelycpp/releases) or build from source:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make deb
sudo dpkg -i shapelycpp-dev-*.deb
```

Headers are installed to `/usr/include/shapelycpp/` along with CMake config. Consuming projects use:

```cmake
find_package(shapelycpp REQUIRED)
target_link_libraries(myapp PRIVATE shapelycpp::shapelycpp)
```

**Manual (header-only)**

Add `-Ipath/to/shapelycpp` to your compiler flags and include the headers directly.

## Project Structure

```
shapelycpp/
├── geometry/           # shapely.geometry.* equivalents
│   ├── point.h
│   ├── point_impl.h
│   ├── linestring.h
│   ├── linestring_impl.h
│   ├── polygon.h
│   └── polygon_impl.h
├── ops/                # shapely.ops.* equivalents
│   ├── nearest_points.h
│   ├── nearest_points_impl.h
│   ├── distance_to_multigeom.h
│   └── distance_to_multigeom_impl.h
├── example/            # usage examples
│   ├── CMakeLists.txt
│   └── main.cpp
├── CMakeLists.txt      # build & .deb packaging
└── README.md
```

## License

[MIT](LICENSE)
