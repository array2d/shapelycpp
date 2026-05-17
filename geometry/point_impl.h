// Python Source: shapely/geometry/point.py
// Line Range: class Point method implementations
// Alignment: strict

#pragma once

#include "shapely/geometry/linestring.h"
#include "shapely/geometry/polygon.h"

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/operation/distance/DistanceOp.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

template <typename T>
Point<T>::Point(T x, T y)
    : x_(x), y_(y)
{
    factory_ = geos::geom::GeometryFactory::create();
    geos_point_.reset(factory_->createPoint(
        geos::geom::Coordinate(static_cast<double>(x), static_cast<double>(y))));
}

template <typename T>
double Point<T>::distance(const Point& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_point_.get(), other.geos_point_.get());
    return dist_op.distance();
}

template <typename T>
template <typename U>
double Point<T>::distance(const LineString<U>& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_point_.get(), other.geos_linestring_.get());
    return dist_op.distance();
}

template <typename T>
template <typename U>
double Point<T>::distance(const Polygon<U>& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_point_.get(), other.geos_polygon_.get());
    return dist_op.distance();
}

template <typename T>
Polygon<double> Point<T>::buffer(double distance) const
{
    auto buf_geom = geos_point_->buffer(distance);
    if (buf_geom == nullptr || buf_geom->isEmpty())
        return std::move(Polygon<double>(py::array_t<double>(std::vector<py::ssize_t>{0, 2})));

    const geos::geom::Geometry *poly = buf_geom.get();
    if (poly->getGeometryTypeId() != geos::geom::GEOS_POLYGON)
    {
        if (poly->getNumGeometries() > 0)
            poly = poly->getGeometryN(0);
    }

    if (poly->getGeometryTypeId() != geos::geom::GEOS_POLYGON || poly->isEmpty())
        return std::move(Polygon<double>(py::array_t<double>(std::vector<py::ssize_t>{0, 2})));

    const geos::geom::Polygon *geos_poly = dynamic_cast<const geos::geom::Polygon *>(poly);
    if (!geos_poly)
        return std::move(Polygon<double>(py::array_t<double>(std::vector<py::ssize_t>{0, 2})));

    const geos::geom::CoordinateSequence *cs = geos_poly->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty())
        return std::move(Polygon<double>(py::array_t<double>(std::vector<py::ssize_t>{0, 2})));

    size_t n = cs->getSize();
    py::array_t<double> coords(std::vector<py::ssize_t>{static_cast<py::ssize_t>(n - 1), static_cast<py::ssize_t>(2)});
    double *p = static_cast<double *>(coords.request().ptr);
    for (size_t i = 0; i < n - 1; ++i) {
        p[i * 2] = cs->getAt(i).x;
        p[i * 2 + 1] = cs->getAt(i).y;
    }

    return std::move(Polygon<double>(coords));
}

} // namespace geometry
} // namespace shapely
