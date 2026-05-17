// Python Source: shapely/geometry/polygon.py
// Line Range: class Polygon method implementations
// Alignment: strict

#pragma once

#include "shapely/geometry/linestring.h"

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/util/TopologyException.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

template <typename T>
Polygon<T>::Polygon(const py::array_t<T>& coords)
    : coords_(coords)
{
    auto buf = coords.request();
    factory_ = geos::geom::GeometryFactory::create();
    if (buf.ndim != 2 || buf.shape[1] < 2 || buf.shape[0] < 3)
    {
        geos_polygon_ = factory_->createPolygon();
        return;
    }

    T *p = static_cast<T *>(buf.ptr);
    size_t n = buf.shape[0];

    auto coord_seq = factory_->getCoordinateSequenceFactory()->create(n + 1, 2);
    for (size_t i = 0; i < n; ++i)
        coord_seq->setAt(geos::geom::Coordinate(
            static_cast<double>(p[i * 2]),
            static_cast<double>(p[i * 2 + 1])), i);
    coord_seq->setAt(geos::geom::Coordinate(
        static_cast<double>(p[0]),
        static_cast<double>(p[1])), n);

    auto ring = factory_->createLinearRing(std::move(coord_seq));
    geos_polygon_ = factory_->createPolygon(std::move(ring));
}

template <typename T>
double Polygon<T>::area() const
{
    return geos_polygon_->getArea();
}

template <typename T>
double Polygon<T>::distance(const Polygon& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_polygon_.get(), other.geos_polygon_.get());
    return dist_op.distance();
}

template <typename T>
template <typename U>
double Polygon<T>::distance(const LineString<U>& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_polygon_.get(), other.geos_linestring_.get());
    return dist_op.distance();
}

template <typename T>
bool Polygon<T>::intersects(const Polygon& other) const
{
    return geos_polygon_->intersects(other.geos_polygon_.get());
}

template <typename T>
template <typename U>
bool Polygon<T>::intersects(const LineString<U>& other) const
{
    return geos_polygon_->intersects(other.geos_linestring_.get());
}

template <typename T>
Polygon<double> Polygon<T>::intersection(const Polygon<double>& other) const
{
    auto empty_result = py::array_t<double>(std::vector<py::ssize_t>{0, 2});

    if (geos_polygon_->isEmpty() || other.geos_polygon_->isEmpty())
        return Polygon<double>(empty_result);

    std::unique_ptr<geos::geom::Geometry> inter;
    if (!geos_polygon_->isValid() || !other.geos_polygon_->isValid()) {
        return Polygon<double>(empty_result);
    }
    inter = geos_polygon_->intersection(other.geos_polygon_.get());
    if (inter == nullptr || inter->isEmpty())
        return Polygon<double>(empty_result);

    const geos::geom::Geometry *geom = inter.get();
    if (geom->getGeometryTypeId() == geos::geom::GEOS_POLYGON)
    {
        const geos::geom::Polygon *geos_poly = dynamic_cast<const geos::geom::Polygon *>(geom);
        if (geos_poly)
        {
            const geos::geom::CoordinateSequence *cs = geos_poly->getExteriorRing()->getCoordinatesRO();
            if (cs && !cs->isEmpty())
            {
                size_t n = cs->getSize();
                py::array_t<double> result(std::vector<py::ssize_t>{static_cast<py::ssize_t>(n - 1), 2});
                double *p = static_cast<double *>(result.request().ptr);
                for (size_t i = 0; i < n - 1; ++i) {
                    p[i * 2] = cs->getAt(i).x;
                    p[i * 2 + 1] = cs->getAt(i).y;
                }
                return Polygon<double>(result);
            }
        }
    }

    return Polygon<double>(empty_result);
}

template <typename T>
double Polygon<T>::intersection_area(const Polygon<double>& other) const
{
    auto inter = intersection(other);
    if (inter.is_empty())
        return 0.0;
    return inter.area();
}

template <typename T>
py::array_t<T> Polygon<T>::exterior_coords() const
{
    const geos::geom::CoordinateSequence *cs = geos_polygon_->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty())
    {
        py::array_t<T> empty(std::vector<py::ssize_t>{0, 2});
        return empty;
    }
    size_t n = cs->getSize();
    py::array_t<T> result(std::vector<py::ssize_t>{static_cast<py::ssize_t>(n - 1), static_cast<py::ssize_t>(2)});
    T *p = static_cast<T *>(result.request().ptr);
    for (size_t i = 0; i < n - 1; ++i) {
        p[i * 2] = static_cast<T>(cs->getAt(i).x);
        p[i * 2 + 1] = static_cast<T>(cs->getAt(i).y);
    }
    return result;
}

template <typename T>
bool Polygon<T>::is_valid() const
{
    if (geos_polygon_->isEmpty())
        return false;
    return geos_polygon_->isValid();
}

template <typename T>
bool Polygon<T>::is_empty() const
{
    return geos_polygon_->isEmpty();
}

template <typename T>
Polygon<double> Polygon<T>::buffer(double distance) const
{
    if (geos_polygon_->isEmpty())
        return std::move(Polygon<double>(py::array_t<double>(std::vector<py::ssize_t>{0, 2})));

    std::unique_ptr<geos::geom::Geometry> buf_geom;
    if (!geos_polygon_->isValid() || !geos_polygon_->isSimple()) {
        return std::move(Polygon<double>(py::array_t<double>(std::vector<py::ssize_t>{0, 2})));
    }
    buf_geom = geos_polygon_->buffer(distance);
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
