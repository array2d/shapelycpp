// Python Source: shapely/geometry/linestring.py
// Line Range: class LineString method implementations
// Alignment: strict

#pragma once

#include "shapely/geometry/point.h"
#include "shapely/geometry/polygon.h"

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/linearref/LengthIndexedLine.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

template <typename T>
LineString<T>::LineString(const py::array_t<T>& coords)
    : coords_(coords)
{
    auto buf = coords.request();
    if (buf.ndim != 2 || buf.shape[1] < 2)
        throw std::runtime_error("LineString: coords must be [N, 2] or [N, 3] array");
    if (buf.shape[0] < 2)
        throw std::runtime_error("LineString: must have at least 2 points");

    factory_ = geos::geom::GeometryFactory::create();
    T *p = static_cast<T *>(buf.ptr);
    size_t n = buf.shape[0];
    int stride = buf.shape[1];

    auto coord_seq = factory_->getCoordinateSequenceFactory()->create(n, 2);
    for (size_t i = 0; i < n; ++i)
        coord_seq->setAt(geos::geom::Coordinate(
            static_cast<double>(p[i * stride + 0]),
            static_cast<double>(p[i * stride + 1])), i);

    geos_linestring_ = factory_->createLineString(std::move(coord_seq));
}

template <typename T>
double LineString<T>::distance(const LineString& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_linestring_.get(), other.geos_linestring_.get());
    return dist_op.distance();
}

template <typename T>
template <typename U>
double LineString<T>::distance(const Polygon<U>& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_linestring_.get(), other.geos_polygon_.get());
    return dist_op.distance();
}

template <typename T>
template <typename U>
double LineString<T>::distance(const Point<U>& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_linestring_.get(), other.geos_point_.get());
    return dist_op.distance();
}

template <typename T>
template <typename U>
bool LineString<T>::intersects(const Polygon<U>& other) const
{
    return geos_linestring_->intersects(other.geos_polygon_.get());
}

template <typename T>
template <typename U>
double LineString<T>::project(const Point<U>& other) const
{
    geos::linearref::LengthIndexedLine indexed_line(geos_linestring_.get());
    return indexed_line.project(geos::geom::Coordinate(
        static_cast<double>(other.x_), static_cast<double>(other.y_)));
}

template <typename T>
Point<double> LineString<T>::interpolate(double distance) const
{
    geos::linearref::LengthIndexedLine indexed_line(geos_linestring_.get());
    auto coord = indexed_line.extractPoint(distance);
    return Point<double>(coord.x, coord.y);
}

template <typename T>
Polygon<double> LineString<T>::buffer(double distance) const
{
    if (!geos_linestring_ || geos_linestring_->isEmpty()) {
        return std::move(Polygon<double>(py::array_t<double>(std::vector<py::ssize_t>{0, 2})));
    }
    auto buf_geom = geos_linestring_->buffer(distance, 16);
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

template <typename T>
double LineString<T>::length() const
{
    return geos_linestring_->getLength();
}

} // namespace geometry
} // namespace shapely
