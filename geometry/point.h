// Python Source: shapely/geometry/point.py
// Line Range: class Point(BaseGeometry)
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template to support both float32 and double coordinates.
// Python shapely only has float64; float32 is a C++ zero-copy optimization.

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <memory>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>

namespace py = pybind11;

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_LINESTRING_DEFINED
template <typename T>
class LineString;
#endif

#ifndef SHAPELY_GEOMETRY_POLYGON_DEFINED
template <typename T>
class Polygon;
#endif

template <typename T = double>
class Point {
public:
    Point(T x, T y);

    Point(Point&&) = default;
    Point& operator=(Point&&) = default;

    double distance(const Point& other) const;

    template <typename U>
    double distance(const LineString<U>& other) const;

    template <typename U>
    double distance(const Polygon<U>& other) const;

    Polygon<double> buffer(double distance) const;

    T x_, y_;

private:
    template <typename U> friend class LineString;
    template <typename U> friend class Polygon;
    std::unique_ptr<geos::geom::Point> geos_point_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_POINT_DEFINED

#include "point_impl.h"
