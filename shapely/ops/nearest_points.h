// Python Source: shapely/ops.py
// Line Range: L328-L355
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template to support both float32 and double coordinates.

#pragma once

#include <tuple>

namespace shapely {
namespace geometry {

template <typename T> class Polygon;
template <typename T> class LineString;
template <typename T> class Point;

} // namespace geometry

namespace ops {

template <typename T>
std::tuple<double, double, double, double> nearest_points(
    const geometry::Polygon<T>& poly,
    const geometry::LineString<T>& line);

template <typename T>
std::tuple<double, double, double, double> nearest_points(
    const geometry::LineString<T>& line,
    const geometry::Point<T>& point);

} // namespace ops
} // namespace shapely

// ============================================================================
// Implementation
// ============================================================================

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

// Python: shapely/ops.py::nearest_points:L328

template <typename T>
std::tuple<double, double, double, double> nearest_points(
    const geometry::Polygon<T>& poly,
    const geometry::LineString<T>& line)
{
    geos::geom::GeometryFactory::Ptr factory = geos::geom::GeometryFactory::create();

    const T *poly_ptr = poly.data();
    size_t poly_n = poly.rows();

    auto poly_coords = factory->getCoordinateSequenceFactory()->create(poly_n + 1, 2);
    for (size_t i = 0; i < poly_n; ++i)
        poly_coords->setAt(geos::geom::Coordinate(
            static_cast<double>(poly_ptr[i * 2]),
            static_cast<double>(poly_ptr[i * 2 + 1])), i);
    poly_coords->setAt(geos::geom::Coordinate(
        static_cast<double>(poly_ptr[0]),
        static_cast<double>(poly_ptr[1])), poly_n);
    auto ring = factory->createLinearRing(std::move(poly_coords));
    auto geos_poly = factory->createPolygon(std::move(ring));

    const T *line_ptr = line.data();
    size_t line_n = line.rows();

    auto line_coords = factory->getCoordinateSequenceFactory()->create(line_n, 2);
    for (size_t i = 0; i < line_n; ++i)
        line_coords->setAt(geos::geom::Coordinate(
            static_cast<double>(line_ptr[i * 2]),
            static_cast<double>(line_ptr[i * 2 + 1])), i);
    auto geos_line = factory->createLineString(std::move(line_coords));

    geos::operation::distance::DistanceOp dist_op(geos_poly.get(), geos_line.get());

    auto coords = dist_op.nearestPoints();
    double poly_x = coords->getAt(0).x;
    double poly_y = coords->getAt(0).y;
    double line_x = coords->getAt(1).x;
    double line_y = coords->getAt(1).y;

    return {poly_x, poly_y, line_x, line_y};
}

// Python: shapely/ops.py::nearest_points:L328

template <typename T>
std::tuple<double, double, double, double> nearest_points(
    const geometry::LineString<T>& line,
    const geometry::Point<T>& point)
{
    geos::geom::GeometryFactory::Ptr factory = geos::geom::GeometryFactory::create();

    const T *line_ptr = line.data();
    size_t line_n = line.rows();

    auto line_coords = factory->getCoordinateSequenceFactory()->create(line_n, 2);
    for (size_t i = 0; i < line_n; ++i)
        line_coords->setAt(geos::geom::Coordinate(
            static_cast<double>(line_ptr[i * 2]),
            static_cast<double>(line_ptr[i * 2 + 1])), i);
    auto geos_line = factory->createLineString(std::move(line_coords));

    auto geos_point = factory->createPoint(geos::geom::Coordinate(
        static_cast<double>(point.x), static_cast<double>(point.y)));

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
