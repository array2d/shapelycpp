// Python Source: shapely/ops.py
// Line Range: utility functions for multi-geometry distance
// Alignment: strict

#pragma once

#include "geometry/polygon.h"
#include "geometry/linestring.h"

#include <limits>

namespace shapely {
namespace ops {

template <typename T>
double distance_to_multiline(
    const geometry::Polygon<double>& poly,
    const std::vector<py::array_t<T>>& lines,
    double target_range)
{
    if (lines.empty())
        return target_range;

    double min_dist = std::numeric_limits<double>::infinity();

    for (const auto& line_coords : lines) {
        geometry::LineString<T> ls(line_coords);
        double dist = poly.distance(ls);
        if (dist < min_dist)
            min_dist = dist;
    }

    return min_dist;
}

template <typename T>
double distance_to_multipolygon(
    const geometry::Polygon<double>& poly,
    const std::vector<py::array_t<T>>& polygons,
    double target_range)
{
    if (polygons.empty())
        return target_range;

    double min_dist = std::numeric_limits<double>::infinity();

    for (const auto& other_coords : polygons) {
        geometry::Polygon<T> other(other_coords);
        double dist = poly.distance(other);
        if (dist < min_dist)
            min_dist = dist;
    }

    return min_dist;
}

template <typename T>
std::pair<double, int> distance_to_multiline_with_index(
    const geometry::Polygon<double>& poly,
    const std::vector<py::array_t<T>>& lines,
    double target_range)
{
    if (lines.empty())
        return {target_range, -1};

    double min_dist = std::numeric_limits<double>::infinity();
    int min_index = -1;

    for (size_t i = 0; i < lines.size(); ++i) {
        geometry::LineString<T> ls(lines[i]);
        double dist = poly.distance(ls);
        if (dist < min_dist) {
            min_dist = dist;
            min_index = static_cast<int>(i);
        }
    }

    if (min_dist >= target_range)
        min_dist = target_range;

    return {min_dist, min_index};
}

} // namespace ops
} // namespace shapely
