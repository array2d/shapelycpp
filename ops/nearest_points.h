// Python Source: shapely/ops.py
// Line Range: def nearest_points (approx)
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template to support both float32 and double coordinates.

#pragma once

#include <tuple>

namespace shapely {
namespace geometry {

template <typename T> class Polygon;
template <typename T> class LineString;
template <typename T> class Point;

} // namespace geometry

namespace ops {

template <typename T>
std::tuple<double, double, double, double> nearest_points(
    const geometry::Polygon<T>& poly,
    const geometry::LineString<T>& line);

template <typename T>
std::tuple<double, double, double, double> nearest_points(
    const geometry::LineString<T>& line,
    const geometry::Point<T>& point);

} // namespace ops
} // namespace shapely

#include "nearest_points_impl.h"
