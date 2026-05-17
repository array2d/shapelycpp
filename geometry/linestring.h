// Python Source: shapely/geometry/linestring.py
// Line Range: class LineString(BaseGeometry)
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template to support both float32 and double coordinates.
// Python shapely only has float64; float32 is a C++ zero-copy optimization.

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <memory>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>

namespace py = pybind11;

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_POLYGON_DEFINED
template <typename T>
class Polygon;
#endif

#ifndef SHAPELY_GEOMETRY_POINT_DEFINED
template <typename T>
class Point;
#endif

template <typename T = double>
class LineString {
public:
    explicit LineString(const py::array_t<T>& coords);

    LineString(LineString&&) = default;
    LineString& operator=(LineString&&) = default;

    double distance(const LineString& other) const;
    template <typename U>
    double distance(const Polygon<U>& other) const;

    template <typename U>
    double distance(const Point<U>& other) const;

    template <typename U>
    bool intersects(const Polygon<U>& other) const;

    template <typename U>
    double project(const Point<U>& other) const;

    Point<double> interpolate(double distance) const;

    double length() const;

    py::array_t<T> coords_;

private:
    template <typename U> friend class Polygon;
    template <typename U> friend class Point;
    std::unique_ptr<geos::geom::LineString> geos_linestring_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_LINESTRING_DEFINED

#include "linestring_impl.h"
