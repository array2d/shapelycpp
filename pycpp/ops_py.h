// Pybind11 wrappers for shapelycpp ops (nearest_points).
//
// Thin layer matching shapely.ops Python API signatures.

#pragma once

#include <pybind11/pybind11.h>
#include <tuple>
#include "../shapely/geometry/polygon.h"
#include "../shapely/geometry/linestring.h"
#include "../shapely/geometry/point.h"
#include "../shapely/ops/nearest_points.h"

namespace py = pybind11;

namespace shapely_py {

/// shapely.ops.nearest_points(poly, line) → ((px,py), (lx,ly))
inline std::tuple<double, double, double, double>
nearest_points_poly_ls(const shapely::geometry::Polygon<double>& poly,
                       const shapely::geometry::LineString<double>& line) {
    return shapely::ops::nearest_points(poly, line);
}

/// shapely.ops.nearest_points(line, point) → ((lx,ly), (px,py))
inline std::tuple<double, double, double, double>
nearest_points_ls_pt(const shapely::geometry::LineString<double>& line,
                     const shapely::geometry::Point<double>& point) {
    return shapely::ops::nearest_points(line, point);
}

} // namespace shapely_py
