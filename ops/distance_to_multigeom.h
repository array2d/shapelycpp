// Python Source: shapely/ops.py
// Line Range: utility functions for multi-geometry distance
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template to support both float32 and double input arrays.

#pragma once

#include <vector>
#include <utility>
#include <pybind11/numpy.h>

namespace py = pybind11;

namespace shapely {
namespace geometry {

template <typename T> class Polygon;
template <typename T> class LineString;

} // namespace geometry

namespace ops {

template <typename T>
double distance_to_multiline(
    const geometry::Polygon<double>& poly,
    const std::vector<py::array_t<T>>& lines,
    double target_range = 200.0);

template <typename T>
double distance_to_multipolygon(
    const geometry::Polygon<double>& poly,
    const std::vector<py::array_t<T>>& polygons,
    double target_range = 200.0);

template <typename T>
std::pair<double, int> distance_to_multiline_with_index(
    const geometry::Polygon<double>& poly,
    const std::vector<py::array_t<T>>& lines,
    double target_range = 200.0);

} // namespace ops
} // namespace shapely

#include "distance_to_multigeom_impl.h"
