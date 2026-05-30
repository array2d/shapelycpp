// Python Source: shapely/geometry/point.py
// Line Range: L16-L194 (class Point + PointAdapter)
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

    // -- Boundary (Python: .boundary) --
    std::string boundary() const;

    // -- Minimum clearance (Python: .minimum_clearance) --
    double minimum_clearance() const;

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
    bool is_closed() const;
    bool is_ring() const;
    double area() const;
    double length() const;
    std::vector<double> bounds() const;

    // -- Constructive operations (Python: difference, union, intersection, sym_difference, simplify) --
    Point<double> difference(const Point& other) const;
    template <typename U> std::string difference(const LineString<U>& other) const;
    template <typename U> std::string difference(const Polygon<U>& other) const;
    Point<double> intersection(const Point& other) const;
    template <typename U> std::string intersection(const LineString<U>& other) const;
    template <typename U> std::string intersection(const Polygon<U>& other) const;
    Point<double> union_op(const Point& other) const;
    template <typename U> std::string union_op(const LineString<U>& other) const;
    template <typename U> std::string union_op(const Polygon<U>& other) const;
    Point<double> symmetric_difference(const Point& other) const;
    template <typename U> std::string symmetric_difference(const LineString<U>& other) const;
    template <typename U> std::string symmetric_difference(const Polygon<U>& other) const;
    Point<double> simplify(double tolerance) const;

    // -- Topology (Python: convex_hull, boundary, envelope, representative_point) --
    Point<double> convex_hull() const;
    Point<double> envelope() const;
    Point<double> representative_point() const;

    // -- Centroid, normalize --
    Point<double> centroid() const;
    void normalize();

private:
    template <typename U> friend class LineString;
    template <typename U> friend class Polygon;
    template <typename U> friend class Point;
    template <typename U> friend class MultiPoint;
    template <typename U> friend class MultiLineString;
    template <typename U> friend class MultiPolygon;
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
#include "shapely/geometry/base.h"

#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/operation/distance/DistanceOp.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

// Python: shapely/geometry/point.py::__init__:L38
// -- Constructor -------------------------------------------------------------

template <typename T>
Point<T>::Point(T px, T py) : x(px), y(py) {
    factory_ = geos::geom::GeometryFactory::create();
    geos_point_.reset(factory_->createPoint(
        geos::geom::Coordinate(static_cast<double>(px), static_cast<double>(py))));
}

// Python: shapely/geometry/point.py::_get_coords:L160
// -- coords, xy -------------------------------------------------------------

template <typename T>
std::vector<std::tuple<T, T>> Point<T>::coords() const {
    return {std::make_tuple(x, y)};
}

// Python: shapely/geometry/point.py::xy:L182
template <typename T>
std::tuple<std::vector<T>, std::vector<T>> Point<T>::xy() const {
    return {{x}, {y}};
}

// Python: shapely/geometry/base.py::distance:L438
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

// Python: shapely/geometry/base.py::buffer:L541
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

// Python: shapely/geometry/base.py::boundary:L457
template <typename T>
std::string Point<T>::boundary() const {
    // Point has no boundary — returns empty geometry
    return "GEOMETRYCOLLECTION EMPTY";
}

// Python: shapely/geometry/base.py::minimum_clearance:L734
template <typename T>
double Point<T>::minimum_clearance() const {
    return detail::geos_minimum_clearance(geos_point_.get());
}

// Python: shapely/geometry/base.py predicates L753-L813
// -- Predicates (macros to reduce boilerplate) -------------------------------

#define SHAPELY_PRED_IMPL(METHOD, GEOS_FN) \
template <typename T> template <typename U> \
bool Point<T>::METHOD(const Point<U>& o) const { return detail::GEOS_FN(geos_point_.get(), o.geos_point_.get()); } \
template <typename T> template <typename U> \
bool Point<T>::METHOD(const LineString<U>& o) const { return detail::GEOS_FN(geos_point_.get(), o.geos_linestring_.get()); } \
template <typename T> template <typename U> \
bool Point<T>::METHOD(const Polygon<U>& o) const { return detail::GEOS_FN(geos_point_.get(), o.geos_polygon_.get()); }

// Python: base.py::contains:L766, within:L813, crosses:L770, disjoint:L774
// Python: base.py::overlaps:L805, touches:L809, covers:L758, covered_by:L762, equals:L778
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

// Python: shapely/geometry/base.py::equals_exact:L817
// equals_exact
template <typename T> template <typename U>
bool Point<T>::equals_exact(const Point<U>& o, double tol) const { return detail::geos_equals_exact(geos_point_.get(), o.geos_point_.get(), tol); }
template <typename T> template <typename U>
bool Point<T>::equals_exact(const LineString<U>& o, double tol) const { return detail::geos_equals_exact(geos_point_.get(), o.geos_linestring_.get(), tol); }
template <typename T> template <typename U>
bool Point<T>::equals_exact(const Polygon<U>& o, double tol) const { return detail::geos_equals_exact(geos_point_.get(), o.geos_polygon_.get(), tol); }

// Python: shapely/geometry/base.py::intersects:L801
// intersects
template <typename T> bool Point<T>::intersects(const Point& o) const { return geos_point_->intersects(o.geos_point_.get()); }
template <typename T> template <typename U> bool Point<T>::intersects(const LineString<U>& o) const { return geos_point_->intersects(o.geos_linestring_.get()); }
template <typename T> template <typename U> bool Point<T>::intersects(const Polygon<U>& o) const { return geos_point_->intersects(o.geos_polygon_.get()); }

// Python: shapely/geometry/base.py::relate:L753, relate_pattern:L890
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

// Python: shapely/geometry/base.py::hausdorff_distance:L442
// -- hausdorff_distance -----------------------------------------------------

template <typename T> template <typename U>
double Point<T>::hausdorff_distance(const Point<U>& o) const { return detail::geos_hausdorff_distance(geos_point_.get(), o.geos_point_.get()); }
template <typename T> template <typename U>
double Point<T>::hausdorff_distance(const LineString<U>& o) const { return detail::geos_hausdorff_distance(geos_point_.get(), o.geos_linestring_.get()); }
template <typename T> template <typename U>
double Point<T>::hausdorff_distance(const Polygon<U>& o) const { return detail::geos_hausdorff_distance(geos_point_.get(), o.geos_polygon_.get()); }

// Python: shapely/geometry/base.py accessors L365-L447
// -- Accessors ---------------------------------------------------------------

// Python: base.py::wkt:L369
template <typename T> std::string Point<T>::wkt() const { return detail::geos_to_wkt(geos_point_.get()); }
// Python: base.py::wkb_hex:L379
template <typename T> std::string Point<T>::wkb_hex() const { return detail::geos_to_wkb_hex(geos_point_.get()); }
// Python: base.py::type:L365
template <typename T> std::string Point<T>::type() const { return "Point"; }
// Python: base.py::geom_type:L426
template <typename T> std::string Point<T>::geom_type() const { return detail::geos_geom_type(geos_point_.get()); }
// Python: base.py::has_z:L708
template <typename T> bool        Point<T>::has_z() const { return detail::geos_has_z(geos_point_.get()); }

// Python: shapely/geometry/base.py properties L714-L470
// -- Properties --------------------------------------------------------------

// Python: base.py::is_empty:L714
template <typename T> bool        Point<T>::is_empty() const { return detail::geos_is_empty(geos_point_.get()); }
// Python: base.py::is_simple:L739
template <typename T> bool        Point<T>::is_simple() const { return detail::geos_is_simple(geos_point_.get()); }
// Python: base.py::is_valid:L745
template <typename T> bool        Point<T>::is_valid() const { return detail::geos_is_valid(geos_point_.get()); }
// Python: base.py::area:L434
template <typename T> double      Point<T>::area() const { return 0.0; }
// Python: base.py::length:L447
template <typename T> double      Point<T>::length() const { return 0.0; }
// Python: base.py::bounds:L470
template <typename T> std::vector<double> Point<T>::bounds() const { return detail::geos_bounds(geos_point_.get()); }

// Python: shapely/geometry/base.py L553-L703
// -- Constructive operations (difference, union, intersection, sym_difference) --

template <typename T>
Point<double> Point<T>::difference(const Point& o) const {
    auto res = detail::geos_difference(geos_point_.get(), o.geos_point_.get());
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (!pt) return Point<double>(0, 0);
    return Point<double>(pt->getX(), pt->getY());
}
template <typename T> template <typename U>
std::string Point<T>::difference(const LineString<U>& o) const {
    auto res = detail::geos_difference(geos_point_.get(), o.geos_linestring_.get());
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}
template <typename T> template <typename U>
std::string Point<T>::difference(const Polygon<U>& o) const {
    auto res = detail::geos_difference(geos_point_.get(), o.geos_polygon_.get());
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}

template <typename T>
Point<double> Point<T>::intersection(const Point& o) const {
    auto res = detail::geos_intersection(geos_point_.get(), o.geos_point_.get());
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (!pt) return Point<double>(0, 0);
    return Point<double>(pt->getX(), pt->getY());
}
template <typename T> template <typename U>
std::string Point<T>::intersection(const LineString<U>& o) const {
    auto res = detail::geos_intersection(geos_point_.get(), o.geos_linestring_.get());
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}
template <typename T> template <typename U>
std::string Point<T>::intersection(const Polygon<U>& o) const {
    auto res = detail::geos_intersection(geos_point_.get(), o.geos_polygon_.get());
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}

template <typename T>
Point<double> Point<T>::union_op(const Point& o) const {
    auto res = detail::geos_union(geos_point_.get(), o.geos_point_.get());
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (!pt) return Point<double>(0, 0);
    return Point<double>(pt->getX(), pt->getY());
}
template <typename T> template <typename U>
std::string Point<T>::union_op(const LineString<U>& o) const {
    auto res = detail::geos_union(geos_point_.get(), o.geos_linestring_.get());
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}
template <typename T> template <typename U>
std::string Point<T>::union_op(const Polygon<U>& o) const {
    auto res = detail::geos_union(geos_point_.get(), o.geos_polygon_.get());
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}

template <typename T>
Point<double> Point<T>::symmetric_difference(const Point& o) const {
    auto res = detail::geos_sym_difference(geos_point_.get(), o.geos_point_.get());
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(res.get());
    if (mpt && mpt->getNumGeometries() > 0) {
        auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(0));
        if (pt) return Point<double>(pt->getX(), pt->getY());
    }
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (pt) return Point<double>(pt->getX(), pt->getY());
    return Point<double>(0, 0);
}
template <typename T> template <typename U>
std::string Point<T>::symmetric_difference(const LineString<U>& o) const {
    auto res = detail::geos_sym_difference(geos_point_.get(), o.geos_linestring_.get());
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}
template <typename T> template <typename U>
std::string Point<T>::symmetric_difference(const Polygon<U>& o) const {
    auto res = detail::geos_sym_difference(geos_point_.get(), o.geos_polygon_.get());
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}

// Python: shapely/geometry/base.py::simplify:L469
template <typename T>
Point<double> Point<T>::simplify(double tol) const {
    auto res = detail::geos_simplify(geos_point_.get(), tol);
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (!pt) return Point<double>(0, 0);
    return Point<double>(pt->getX(), pt->getY());
}

// Python: shapely/geometry/base.py::convex_hull:L567, boundary:L457, envelope:L742
template <typename T>
Point<double> Point<T>::convex_hull() const {
    auto res = detail::geos_convex_hull(geos_point_.get());
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (!pt) return Point<double>(0, 0);
    return Point<double>(pt->getX(), pt->getY());
}

template <typename T>
Point<double> Point<T>::envelope() const {
    auto res = detail::geos_envelope(geos_point_.get());
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (!pt) return Point<double>(0, 0);
    return Point<double>(pt->getX(), pt->getY());
}

template <typename T>
Point<double> Point<T>::representative_point() const {
    auto res = detail::geos_representative_point(geos_point_.get());
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (!pt) return Point<double>(0, 0);
    return Point<double>(pt->getX(), pt->getY());
}

// Python: shapely/geometry/base.py::centroid:L478
// -- Centroid ----------------------------------------------------------------

template <typename T>
Point<double> Point<T>::centroid() const {
    auto c = geos_point_->getCentroid();
    if (!c) return Point<double>(0, 0);
    auto* coord = c->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

// Python: shapely/geometry/base.py::normalize:L663
// -- Normalize ---------------------------------------------------------------

template <typename T>
void Point<T>::normalize() { geos_point_->normalize(); }

} // namespace geometry
} // namespace shapely
