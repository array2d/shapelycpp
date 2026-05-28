// Python Source: shapely/geometry/point.py
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template for float32/float64 coordinate support.

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <tuple>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_LINESTRING_DEFINED
template <typename T> class LineString;
#endif
#ifndef SHAPELY_GEOMETRY_POLYGON_DEFINED
template <typename T> class Polygon;
#endif

template <typename T = double>
class Point {
public:
    Point(T px, T py);
    Point(Point&&) = default;
    Point& operator=(Point&&) = default;

    // -- Coordinates (Python: .x, .y, .coords, .xy) --
    T x, y;
    std::vector<std::tuple<T, T>> coords() const;
    std::tuple<std::vector<T>, std::vector<T>> xy() const;

    // -- Distance (Python: distance) --
    double distance(const Point& other) const;
    template <typename U> double distance(const LineString<U>& other) const;
    template <typename U> double distance(const Polygon<U>& other) const;

    // -- Buffer (Python: buffer) --
    Polygon<double> buffer(double distance) const;

    // -- Predicates (Python: contains, within, ...) --
    template <typename U> bool contains(const Point<U>& other) const;
    template <typename U> bool contains(const LineString<U>& other) const;
    template <typename U> bool contains(const Polygon<U>& other) const;
    template <typename U> bool within(const Point<U>& other) const;
    template <typename U> bool within(const LineString<U>& other) const;
    template <typename U> bool within(const Polygon<U>& other) const;
    template <typename U> bool crosses(const Point<U>& other) const;
    template <typename U> bool crosses(const LineString<U>& other) const;
    template <typename U> bool crosses(const Polygon<U>& other) const;
    template <typename U> bool disjoint(const Point<U>& other) const;
    template <typename U> bool disjoint(const LineString<U>& other) const;
    template <typename U> bool disjoint(const Polygon<U>& other) const;
    template <typename U> bool overlaps(const Point<U>& other) const;
    template <typename U> bool overlaps(const LineString<U>& other) const;
    template <typename U> bool overlaps(const Polygon<U>& other) const;
    template <typename U> bool touches(const Point<U>& other) const;
    template <typename U> bool touches(const LineString<U>& other) const;
    template <typename U> bool touches(const Polygon<U>& other) const;
    template <typename U> bool covers(const Point<U>& other) const;
    template <typename U> bool covers(const LineString<U>& other) const;
    template <typename U> bool covers(const Polygon<U>& other) const;
    template <typename U> bool covered_by(const Point<U>& other) const;
    template <typename U> bool covered_by(const LineString<U>& other) const;
    template <typename U> bool covered_by(const Polygon<U>& other) const;
    template <typename U> bool equals(const Point<U>& other) const;
    template <typename U> bool equals(const LineString<U>& other) const;
    template <typename U> bool equals(const Polygon<U>& other) const;
    template <typename U> bool equals_exact(const Point<U>& other, double tol) const;
    template <typename U> bool equals_exact(const LineString<U>& other, double tol) const;
    template <typename U> bool equals_exact(const Polygon<U>& other, double tol) const;

    bool intersects(const Point& other) const;
    template <typename U> bool intersects(const LineString<U>& other) const;
    template <typename U> bool intersects(const Polygon<U>& other) const;

    // -- DE-9IM (Python: relate) --
    template <typename U> std::string relate(const Point<U>& other) const;
    template <typename U> std::string relate(const LineString<U>& other) const;
    template <typename U> std::string relate(const Polygon<U>& other) const;
    template <typename U> bool relate_pattern(const Point<U>& other, const std::string& pattern) const;
    template <typename U> bool relate_pattern(const LineString<U>& other, const std::string& pattern) const;
    template <typename U> bool relate_pattern(const Polygon<U>& other, const std::string& pattern) const;

    // -- Hausdorff distance --
    template <typename U> double hausdorff_distance(const Point<U>& other) const;
    template <typename U> double hausdorff_distance(const LineString<U>& other) const;
    template <typename U> double hausdorff_distance(const Polygon<U>& other) const;

    // -- Accessors (Python: wkt, wkb, wkb_hex, type, geom_type, has_z) --
    std::string wkt() const;
    std::string wkb_hex() const;
    std::string type() const;
    std::string geom_type() const;
    bool has_z() const;

    // -- Geometry properties (Python: is_empty, is_simple, is_valid, area, length, bounds) --
    bool is_empty() const;
    bool is_simple() const;
    bool is_valid() const;
    double area() const;
    double length() const;
    std::vector<double> bounds() const;

    // -- Centroid, normalize --
    Point<double> centroid() const;
    void normalize();

private:
    template <typename U> friend class LineString;
    template <typename U> friend class Polygon;
    template <typename U> friend class Point;
    std::unique_ptr<geos::geom::Point> geos_point_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_POINT_DEFINED

// ============================================================================
// Implementation
// ============================================================================

#include "shapely/geometry/linestring.h"
#include "shapely/geometry/polygon.h"
#include "shapely/detail/geos_utils.h"

#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/operation/distance/DistanceOp.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

// -- Constructor -------------------------------------------------------------

template <typename T>
Point<T>::Point(T px, T py) : x(px), y(py) {
    factory_ = geos::geom::GeometryFactory::create();
    geos_point_.reset(factory_->createPoint(
        geos::geom::Coordinate(static_cast<double>(px), static_cast<double>(py))));
}

// -- coords, xy -------------------------------------------------------------

template <typename T>
std::vector<std::tuple<T, T>> Point<T>::coords() const {
    return {std::make_tuple(x, y)};
}

template <typename T>
std::tuple<std::vector<T>, std::vector<T>> Point<T>::xy() const {
    return {{x}, {y}};
}

// -- distance ----------------------------------------------------------------

template <typename T>
double Point<T>::distance(const Point& other) const {
    geos::operation::distance::DistanceOp op(geos_point_.get(), other.geos_point_.get());
    return op.distance();
}

template <typename T> template <typename U>
double Point<T>::distance(const LineString<U>& other) const {
    geos::operation::distance::DistanceOp op(geos_point_.get(), other.geos_linestring_.get());
    return op.distance();
}

template <typename T> template <typename U>
double Point<T>::distance(const Polygon<U>& other) const {
    geos::operation::distance::DistanceOp op(geos_point_.get(), other.geos_polygon_.get());
    return op.distance();
}

// -- buffer ------------------------------------------------------------------

template <typename T>
Polygon<double> Point<T>::buffer(double distance) const {
    auto buf = geos_point_->buffer(distance, 16);
    if (!buf || buf->isEmpty()) return Polygon<double>();
    const geos::geom::Geometry* poly = buf.get();
    if (poly->getGeometryTypeId() != geos::geom::GEOS_POLYGON) {
        if (poly->getNumGeometries() > 0) poly = poly->getGeometryN(0);
    }
    if (poly->getGeometryTypeId() != geos::geom::GEOS_POLYGON || poly->isEmpty()) return Polygon<double>();
    auto* gp = dynamic_cast<const geos::geom::Polygon*>(poly);
    if (!gp) return Polygon<double>();
    auto* cs = gp->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty()) return Polygon<double>();
    size_t n = cs->getSize();
    std::vector<double> c(n * 2);
    for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
    return Polygon<double>(c.data(), n, 2);
}

// -- Predicates (macros to reduce boilerplate) -------------------------------

#define SHAPELY_PRED_IMPL(METHOD, GEOS_FN) \
template <typename T> template <typename U> \
bool Point<T>::METHOD(const Point<U>& o) const { return detail::GEOS_FN(geos_point_.get(), o.geos_point_.get()); } \
template <typename T> template <typename U> \
bool Point<T>::METHOD(const LineString<U>& o) const { return detail::GEOS_FN(geos_point_.get(), o.geos_linestring_.get()); } \
template <typename T> template <typename U> \
bool Point<T>::METHOD(const Polygon<U>& o) const { return detail::GEOS_FN(geos_point_.get(), o.geos_polygon_.get()); }

SHAPELY_PRED_IMPL(contains,    geos_contains)
SHAPELY_PRED_IMPL(within,      geos_within)
SHAPELY_PRED_IMPL(crosses,     geos_crosses)
SHAPELY_PRED_IMPL(disjoint,    geos_disjoint)
SHAPELY_PRED_IMPL(overlaps,    geos_overlaps)
SHAPELY_PRED_IMPL(touches,     geos_touches)
SHAPELY_PRED_IMPL(covers,      geos_covers)
SHAPELY_PRED_IMPL(covered_by,  geos_covered_by)
SHAPELY_PRED_IMPL(equals,      geos_equals)

#undef SHAPELY_PRED_IMPL

// equals_exact
template <typename T> template <typename U>
bool Point<T>::equals_exact(const Point<U>& o, double tol) const { return detail::geos_equals_exact(geos_point_.get(), o.geos_point_.get(), tol); }
template <typename T> template <typename U>
bool Point<T>::equals_exact(const LineString<U>& o, double tol) const { return detail::geos_equals_exact(geos_point_.get(), o.geos_linestring_.get(), tol); }
template <typename T> template <typename U>
bool Point<T>::equals_exact(const Polygon<U>& o, double tol) const { return detail::geos_equals_exact(geos_point_.get(), o.geos_polygon_.get(), tol); }

// intersects
template <typename T> bool Point<T>::intersects(const Point& o) const { return geos_point_->intersects(o.geos_point_.get()); }
template <typename T> template <typename U> bool Point<T>::intersects(const LineString<U>& o) const { return geos_point_->intersects(o.geos_linestring_.get()); }
template <typename T> template <typename U> bool Point<T>::intersects(const Polygon<U>& o) const { return geos_point_->intersects(o.geos_polygon_.get()); }

// -- relate / relate_pattern ------------------------------------------------

#define SHAPELY_RELATE_IMPL \
template <typename T> template <typename U> \
std::string Point<T>::relate(const Point<U>& o) const { return detail::geos_relate(geos_point_.get(), o.geos_point_.get()); } \
template <typename T> template <typename U> \
std::string Point<T>::relate(const LineString<U>& o) const { return detail::geos_relate(geos_point_.get(), o.geos_linestring_.get()); } \
template <typename T> template <typename U> \
std::string Point<T>::relate(const Polygon<U>& o) const { return detail::geos_relate(geos_point_.get(), o.geos_polygon_.get()); } \
template <typename T> template <typename U> \
bool Point<T>::relate_pattern(const Point<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_point_.get(), o.geos_point_.get(), p); } \
template <typename T> template <typename U> \
bool Point<T>::relate_pattern(const LineString<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_point_.get(), o.geos_linestring_.get(), p); } \
template <typename T> template <typename U> \
bool Point<T>::relate_pattern(const Polygon<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_point_.get(), o.geos_polygon_.get(), p); }

SHAPELY_RELATE_IMPL
#undef SHAPELY_RELATE_IMPL

// -- hausdorff_distance -----------------------------------------------------

template <typename T> template <typename U>
double Point<T>::hausdorff_distance(const Point<U>& o) const { return detail::geos_hausdorff_distance(geos_point_.get(), o.geos_point_.get()); }
template <typename T> template <typename U>
double Point<T>::hausdorff_distance(const LineString<U>& o) const { return detail::geos_hausdorff_distance(geos_point_.get(), o.geos_linestring_.get()); }
template <typename T> template <typename U>
double Point<T>::hausdorff_distance(const Polygon<U>& o) const { return detail::geos_hausdorff_distance(geos_point_.get(), o.geos_polygon_.get()); }

// -- Accessors ---------------------------------------------------------------

template <typename T> std::string Point<T>::wkt() const { return detail::geos_to_wkt(geos_point_.get()); }
template <typename T> std::string Point<T>::wkb_hex() const { return detail::geos_to_wkb_hex(geos_point_.get()); }
template <typename T> std::string Point<T>::type() const { return "Point"; }
template <typename T> std::string Point<T>::geom_type() const { return detail::geos_geom_type(geos_point_.get()); }
template <typename T> bool        Point<T>::has_z() const { return detail::geos_has_z(geos_point_.get()); }

// -- Properties --------------------------------------------------------------

template <typename T> bool        Point<T>::is_empty() const { return detail::geos_is_empty(geos_point_.get()); }
template <typename T> bool        Point<T>::is_simple() const { return detail::geos_is_simple(geos_point_.get()); }
template <typename T> bool        Point<T>::is_valid() const { return detail::geos_is_valid(geos_point_.get()); }
template <typename T> double      Point<T>::area() const { return 0.0; }
template <typename T> double      Point<T>::length() const { return 0.0; }
template <typename T> std::vector<double> Point<T>::bounds() const { return detail::geos_bounds(geos_point_.get()); }

// -- Centroid ----------------------------------------------------------------

template <typename T>
Point<double> Point<T>::centroid() const {
    auto c = geos_point_->getCentroid();
    if (!c) return Point<double>(0, 0);
    auto* coord = c->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

// -- Normalize ---------------------------------------------------------------

template <typename T>
void Point<T>::normalize() { geos_point_->normalize(); }

} // namespace geometry
} // namespace shapely
