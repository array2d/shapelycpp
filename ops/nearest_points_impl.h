// Python Source: shapely/ops.py
// Line Range: def nearest_points (approx)
// Alignment: strict

#pragma once

#include "shapely/geometry/polygon.h"
#include "shapely/geometry/linestring.h"
#include "shapely/geometry/point.h"

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/operation/distance/DistanceOp.h>

namespace shapely {
namespace ops {

template <typename T>
std::tuple<double, double, double, double> nearest_points(
    const geometry::Polygon<T>& poly,
    const geometry::LineString<T>& line)
{
    geos::geom::GeometryFactory::Ptr factory = geos::geom::GeometryFactory::create();

    auto poly_buf = poly.coords_.request();
    T *pp = static_cast<T *>(poly_buf.ptr);
    size_t pn = poly_buf.shape[0];

    auto poly_coords = factory->getCoordinateSequenceFactory()->create(pn + 1, 2);
    for (size_t i = 0; i < pn; ++i)
        poly_coords->setAt(geos::geom::Coordinate(
            static_cast<double>(pp[i * 2]),
            static_cast<double>(pp[i * 2 + 1])), i);
    poly_coords->setAt(geos::geom::Coordinate(
        static_cast<double>(pp[0]),
        static_cast<double>(pp[1])), pn);
    auto ring = factory->createLinearRing(std::move(poly_coords));
    auto geos_poly = factory->createPolygon(std::move(ring));

    auto line_buf = line.coords_.request();
    T *lp = static_cast<T *>(line_buf.ptr);
    size_t ln = line_buf.shape[0];

    auto line_coords = factory->getCoordinateSequenceFactory()->create(ln, 2);
    for (size_t i = 0; i < ln; ++i)
        line_coords->setAt(geos::geom::Coordinate(
            static_cast<double>(lp[i * 2]),
            static_cast<double>(lp[i * 2 + 1])), i);
    auto geos_line = factory->createLineString(std::move(line_coords));

    geos::operation::distance::DistanceOp dist_op(geos_poly.get(), geos_line.get());

    auto coords = dist_op.nearestPoints();
    double poly_x = coords->getAt(0).x;
    double poly_y = coords->getAt(0).y;
    double line_x = coords->getAt(1).x;
    double line_y = coords->getAt(1).y;

    return {poly_x, poly_y, line_x, line_y};
}

template <typename T>
std::tuple<double, double, double, double> nearest_points(
    const geometry::LineString<T>& line,
    const geometry::Point<T>& point)
{
    geos::geom::GeometryFactory::Ptr factory = geos::geom::GeometryFactory::create();

    auto line_buf = line.coords_.request();
    T *lp = static_cast<T *>(line_buf.ptr);
    size_t ln = line_buf.shape[0];

    auto line_coords = factory->getCoordinateSequenceFactory()->create(ln, 2);
    for (size_t i = 0; i < ln; ++i)
        line_coords->setAt(geos::geom::Coordinate(
            static_cast<double>(lp[i * 2]),
            static_cast<double>(lp[i * 2 + 1])), i);
    auto geos_line = factory->createLineString(std::move(line_coords));

    auto geos_point = factory->createPoint(geos::geom::Coordinate(
        static_cast<double>(point.x_), static_cast<double>(point.y_)));

    geos::operation::distance::DistanceOp dist_op(geos_line.get(), geos_point);

    auto coords = dist_op.nearestPoints();
    double line_x = coords->getAt(0).x;
    double line_y = coords->getAt(0).y;
    double point_x = coords->getAt(1).x;
    double point_y = coords->getAt(1).y;

    return {line_x, line_y, point_x, point_y};
}

} // namespace ops
} // namespace shapely
