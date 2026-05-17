// Python Source: shapely/geometry/polygon.py
// Line Range: class Polygon(BaseGeometry)
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template to support both float32 and double coordinates.
// Python shapely only has float64; float32 is a C++ zero-copy optimization.

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <memory>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>

namespace py = pybind11;

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_LINESTRING_DEFINED
template <typename T>
class LineString;
#endif

template <typename T = double>
class Polygon {
public:
    explicit Polygon(const py::array_t<T>& coords);

    Polygon(Polygon&&) = default;
    Polygon& operator=(Polygon&&) = default;

    double area() const;
    double distance(const Polygon& other) const;

    template <typename U>
    double distance(const LineString<U>& other) const;

    bool intersects(const Polygon& other) const;

    template <typename U>
    bool intersects(const LineString<U>& other) const;

    Polygon<double> intersection(const Polygon<double>& other) const;
    double intersection_area(const Polygon<double>& other) const;
    bool is_valid() const;
    bool is_empty() const;
    Polygon<double> buffer(double distance) const;
    py::array_t<T> exterior_coords() const;

    py::array_t<T> coords_;

private:
    template <typename U> friend class Polygon;
    template <typename U> friend class LineString;
    template <typename U> friend class Point;
    std::unique_ptr<geos::geom::Polygon> geos_polygon_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_POLYGON_DEFINED

#include "polygon_impl.h"
