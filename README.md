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

## Precision Alignment

### `double` (C++) ↔ Python shapely: Bit-identical

When using `Point<double>`, `LineString<double>`, `Polygon<double>` (the default), all geometry operations produce **bit-identical** results to Python shapely. This is guaranteed because:

- Both use the same GEOS engine (`geos::operation::distance::DistanceOp`, `Geometry::intersects()`, etc.).
- Python shapely calls `GEOSDistance_r()` (C API), which internally invokes the same `DistanceOp::distance()` as the C++ `shapelycpp`.
- `LinearRing` / `Polygon` / `LineString` constructors both populate `geos::geom::CoordinateSequence` via the same `Coordinate(x, y)` path.

### `float` (C++) vs Python shapely: Bit-identical with input truncation

Python shapely has no `float32` mode — all coordinates are auto-promoted to Python `float` (C `double`). However, **bit-identical results can still be achieved** by truncating inputs to `float32` *before* passing them to both sides:

| Strategy | C++ side | Python side | GEOS internal `double` value | Result |
|----------|----------|-------------|------------------------------|--------|
| Truncate inputs to `float32`, then pass to both | `Point<float>` constructor widens to `double` | `PyPoint(f32_x, f32_y)` receives `float` | **Identical** | **Bit-identical** ✅ |

This works because:
1. Every `float32` value is **exactly representable** in `float64` — no rounding occurs during widening.
2. `np.float32(x)` → `float(np.float32(x))` produces the same `double` value that C++ `Point<float>` uses internally.
3. Both C++ and Python then feed the **identical `double` value** into GEOS's `Coordinate(x, y)` → `CoordinateSequence` → `DistanceOp::distance()` → **same result**.

```python
# Example: float32 bit-alignment strategy
def _f32(coords):
    """Truncate to float32 first so both sides receive identical doubles."""
    if isinstance(coords, tuple):
        return tuple(_f32(list(coords)))
    if isinstance(coords, (int, float)):
        return float(np.float32(coords))
    return [(_f32(c[0]), _f32(c[1])) for c in coords]

# Usage in tests
f32_coords = _f32(original_coords)
cpp_result = cpp_point_f32.distance(cpp_polygon_f32)   # C++ f32 path
py_result  = PyPoint(f32_coords).distance(PyPolygon(f32_coords))  # Python f64 path
assert cpp_result == py_result  # strict equality — bit-identical
```

> **Note**: Without input truncation, C++ `float` results will differ from Python `double` results (up to ~`1e-5` relative error), because the C++ side truncates to `float32` while Python retains full `float64` precision — the two sides feed *different* `double` values into GEOS.

### Collision-critical operations

The following patterns from production collision detection code are covered by exhaustive tests (1,000+ random pairs per pattern, plus deterministic boundary edge cases), all verified with **strict equality** (`==`):

| # | Pattern | Production usage |
|---|---------|-----------------|
| 1 | Polygon ↔ Polygon distance | `bool collision = (distance == 0.0)` |
| 2 | Polygon ↔ LineString distance | `double d = poly.distance(line)` |
| 3 | `intersects` + `distance` | `intersects(g1, g2) && distance(g1, g2) < 1e-12` |
| 4 | LineString ↔ Point distance | `dist = line.distance(ego_pt)` |
| 5 | LineString ↔ LineString distance | `half_lane_width = line1.distance(line2) / 2.0` |
| 6 | LineString ↔ Polygon (both directions) | `centerline.distance(poly)`; `poly.distance(line)` |

Edge cases tested at collision decision boundaries: touching edges/vertices, ε-overlap (`1e-12`), near-touching (`1e-12` gap), point-on-boundary, parallel-near segments, collinear-near segments, and endpoint-on-boundary.

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
├── shapely/            # pure native C++ (zero pybind11 dependency)
│   ├── geometry/
│   │   ├── point.h
│   │   ├── linestring.h
│   │   └── polygon.h
│   └── ops/
│       └── nearest_points.h
├── example/            # native C++ usage examples
│   ├── CMakeLists.txt
│   └── main.cpp
├── tests/              # Python precision alignment tests
│   ├── module.cpp      # pybind11 test module
│   ├── conftest.py     # pytest fixtures (float64/float32 make, CppFactory)
│   ├── utils.py        # coordinate generators & comparison helpers
│   └── test_api.py     # unified precision alignment tests (float64/float32)
├── CMakeLists.txt      # build & .deb packaging
└── README.md
```

## License

[MIT](LICENSE)
