// Python Source: shapely/geometry/point.py
// Line Range: class Point(BaseGeometry) (L15-L238)
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template to support both float32 and double coordinates.

#pragma once

#include <memory>
#include <vector>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>

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
    Point(T px, T py);

    Point(Point&&) = default;
    Point& operator=(Point&&) = default;

    double distance(const Point& other) const;

    template <typename U>
    double distance(const LineString<U>& other) const;

    template <typename U>
    double distance(const Polygon<U>& other) const;

    Polygon<double> buffer(double distance) const;

    T x, y;

private:
    template <typename U> friend class LineString;
    template <typename U> friend class Polygon;
    std::unique_ptr<geos::geom::Point> geos_point_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_POINT_DEFINED

// ============================================================================
// Implementation — requires full LineString and Polygon definitions
// ============================================================================

#include "shapely/geometry/linestring.h"
#include "shapely/geometry/polygon.h"

#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/operation/distance/DistanceOp.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

// Python: shapely/geometry/point.py::Point.__init__:L30

template <typename T>
Point<T>::Point(T px, T py)
    : x(px), y(py)
{
    factory_ = geos::geom::GeometryFactory::create();
    geos_point_.reset(factory_->createPoint(
        geos::geom::Coordinate(static_cast<double>(px), static_cast<double>(py))));
}

// Python: shapely/geometry/point.py::Point.distance:L100

template <typename T>
double Point<T>::distance(const Point& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_point_.get(), other.geos_point_.get());
    return dist_op.distance();
}

// Python: shapely/geometry/point.py::Point.distance:L100

template <typename T>
template <typename U>
double Point<T>::distance(const LineString<U>& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_point_.get(), other.geos_linestring_.get());
    return dist_op.distance();
}

// Python: shapely/geometry/point.py::Point.distance:L100

template <typename T>
template <typename U>
double Point<T>::distance(const Polygon<U>& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_point_.get(), other.geos_polygon_.get());
    return dist_op.distance();
}

// Python: shapely/geometry/point.py::Point.buffer:L79

template <typename T>
Polygon<double> Point<T>::buffer(double distance) const
{
    auto buf_geom = geos_point_->buffer(distance, 16);
    if (buf_geom == nullptr || buf_geom->isEmpty())
        return Polygon<double>();

    const geos::geom::Geometry *poly = buf_geom.get();
    if (poly->getGeometryTypeId() != geos::geom::GEOS_POLYGON)
    {
        if (poly->getNumGeometries() > 0)
            poly = poly->getGeometryN(0);
    }

    if (poly->getGeometryTypeId() != geos::geom::GEOS_POLYGON || poly->isEmpty())
        return Polygon<double>();

    const geos::geom::Polygon *geos_poly = dynamic_cast<const geos::geom::Polygon *>(poly);
    if (!geos_poly)
        return Polygon<double>();

    const geos::geom::CoordinateSequence *cs = geos_poly->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty())
        return Polygon<double>();

    size_t coord_n = cs->getSize();
    std::vector<double> coords(coord_n * 2);
    for (size_t i = 0; i < coord_n; ++i) {
        coords[i * 2]     = cs->getAt(i).x;
        coords[i * 2 + 1] = cs->getAt(i).y;
    }

    return Polygon<double>(coords.data(), coord_n, 2);
}

} // namespace geometry
} // namespace shapely
