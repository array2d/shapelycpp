// Python Source: shapely/geometry/multipolygon.py
// Line Range: L10-L80 (class MultiPolygon + MultiPolygonAdapter)
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template for float32/float64 coordinate support.

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <tuple>
#include <cstddef>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Polygon.h>

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_POLYGON_DEFINED
template <typename T> class Polygon;
#endif
#ifndef SHAPELY_GEOMETRY_POINT_DEFINED
template <typename T> class Point;
#endif
#ifndef SHAPELY_GEOMETRY_LINESTRING_DEFINED
template <typename T> class LineString;
#endif
#ifndef SHAPELY_GEOMETRY_MULTILINESTRING_DEFINED
template <typename T> class MultiLineString;
#endif

template <typename T = double>
class MultiPolygon {
public:
    MultiPolygon();

    /// Add a polygon from raw exterior ring coordinates
    void add_polygon(const T* coords, size_t rows, size_t cols = 2);

    MultiPolygon(MultiPolygon&&) = default;
    MultiPolygon& operator=(MultiPolygon&&) = default;

    // -- Access to individual polygons --
    size_t num_geometries() const;
    Polygon<double> geometry_n(size_t i) const;

    // -- Distance --
    template <typename U> double distance(const Point<U>& other) const;
    template <typename U> double distance(const LineString<U>& other) const;
    template <typename U> double distance(const Polygon<U>& other) const;
    template <typename U> double distance(const MultiLineString<U>& other) const;
    double distance(const MultiPolygon& other) const;

    // -- Predicates --
    template <typename U> bool contains(const Point<U>& other) const;
    template <typename U> bool contains(const LineString<U>& other) const;
    template <typename U> bool contains(const Polygon<U>& other) const;
    bool contains(const MultiPolygon& other) const;
    template <typename U> bool within(const Point<U>& other) const;
    template <typename U> bool within(const LineString<U>& other) const;
    template <typename U> bool within(const Polygon<U>& other) const;
    bool within(const MultiPolygon& other) const;
    template <typename U> bool crosses(const Point<U>& other) const;
    template <typename U> bool crosses(const LineString<U>& other) const;
    template <typename U> bool crosses(const Polygon<U>& other) const;
    template <typename U> bool disjoint(const Point<U>& other) const;
    template <typename U> bool disjoint(const LineString<U>& other) const;
    template <typename U> bool disjoint(const Polygon<U>& other) const;
    bool disjoint(const MultiPolygon& other) const;
    template <typename U> bool overlaps(const Point<U>& other) const;
    template <typename U> bool overlaps(const LineString<U>& other) const;
    template <typename U> bool overlaps(const Polygon<U>& other) const;
    bool overlaps(const MultiPolygon& other) const;
    template <typename U> bool touches(const Point<U>& other) const;
    template <typename U> bool touches(const LineString<U>& other) const;
    template <typename U> bool touches(const Polygon<U>& other) const;
    bool touches(const MultiPolygon& other) const;
    template <typename U> bool covers(const Point<U>& other) const;
    template <typename U> bool covers(const LineString<U>& other) const;
    template <typename U> bool covers(const Polygon<U>& other) const;
    template <typename U> bool covered_by(const Point<U>& other) const;
    template <typename U> bool covered_by(const LineString<U>& other) const;
    template <typename U> bool covered_by(const Polygon<U>& other) const;
    template <typename U> bool equals(const Point<U>& other) const;
    template <typename U> bool equals(const LineString<U>& other) const;
    template <typename U> bool equals(const Polygon<U>& other) const;
    bool equals(const MultiPolygon& other) const;
    template <typename U> bool equals_exact(const Point<U>& other, double tol) const;
    template <typename U> bool equals_exact(const LineString<U>& other, double tol) const;
    template <typename U> bool equals_exact(const Polygon<U>& other, double tol) const;
    bool equals_exact(const MultiPolygon& other, double tol) const;

    bool intersects(const MultiPolygon& other) const;
    template <typename U> bool intersects(const Point<U>& other) const;
    template <typename U> bool intersects(const LineString<U>& other) const;
    template <typename U> bool intersects(const Polygon<U>& other) const;
    template <typename U> bool intersects(const MultiLineString<U>& other) const;

    // -- DE-9IM --
    template <typename U> std::string relate(const Point<U>& other) const;
    template <typename U> std::string relate(const LineString<U>& other) const;
    template <typename U> std::string relate(const Polygon<U>& other) const;
    std::string relate(const MultiPolygon& other) const;
    template <typename U> bool relate_pattern(const Point<U>& other, const std::string& p) const;
    template <typename U> bool relate_pattern(const LineString<U>& other, const std::string& p) const;
    template <typename U> bool relate_pattern(const Polygon<U>& other, const std::string& p) const;
    bool relate_pattern(const MultiPolygon& other, const std::string& p) const;

    // -- Hausdorff distance --
    template <typename U> double hausdorff_distance(const Point<U>& other) const;
    template <typename U> double hausdorff_distance(const LineString<U>& other) const;
    template <typename U> double hausdorff_distance(const Polygon<U>& other) const;
    double hausdorff_distance(const MultiPolygon& other) const;

    // -- Constructive operations --
    MultiPolygon<double> difference(const MultiPolygon<double>& other) const;
    MultiPolygon<double> intersection(const MultiPolygon<double>& other) const;
    MultiPolygon<double> union_op(const MultiPolygon<double>& other) const;
    MultiPolygon<double> symmetric_difference(const MultiPolygon<double>& other) const;
    MultiPolygon<double> simplify(double tolerance) const;

    // -- Accessors --
    std::string wkt() const;
    std::string wkb_hex() const;
    std::string type() const;
    std::string geom_type() const;
    bool has_z() const;

    // -- Properties --
    bool is_empty() const;
    bool is_simple() const;
    bool is_valid() const;
    double area() const;
    double length() const;
    std::vector<double> bounds() const;

    // -- Topology --
    Point<double> centroid() const;
    MultiPolygon<double> convex_hull() const;
    MultiPolygon<double> buffer(double distance) const;
    void normalize();

private:
    template <typename U> friend class Point;
    template <typename U> friend class LineString;
    template <typename U> friend class Polygon;
    template <typename U> friend class MultiLineString;
    std::unique_ptr<geos::geom::MultiPolygon> geos_mp_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_MULTIPOLYGON_DEFINED

// ============================================================================
// Implementation
// ============================================================================

#include "shapely/geometry/point.h"
#include "shapely/geometry/linestring.h"
#include "shapely/geometry/polygon.h"
#include "shapely/geometry/multilinestring.h"
#include "shapely/geometry/base.h"
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
MultiPolygon<T>::MultiPolygon() {
    factory_ = geos::geom::GeometryFactory::create();
    geos_mp_ = factory_->createMultiPolygon();
}

template <typename T>
void MultiPolygon<T>::add_polygon(const T* coords, size_t rows, size_t cols) {
    if (cols < 2 || rows < 3) return;
    auto cs = factory_->getCoordinateSequenceFactory()->create(rows+1, 2);
    for (size_t i = 0; i < rows; ++i)
        cs->setAt(geos::geom::Coordinate(static_cast<double>(coords[i*cols]),
                                          static_cast<double>(coords[i*cols+1])), i);
    cs->setAt(geos::geom::Coordinate(static_cast<double>(coords[0]),
                                      static_cast<double>(coords[1])), rows);
    auto ring = factory_->createLinearRing(std::move(cs));
    auto poly = factory_->createPolygon(std::move(ring));
    std::vector<std::unique_ptr<geos::geom::Geometry>> geoms;
    for (size_t i = 0; i < geos_mp_->getNumGeometries(); ++i)
        geoms.push_back(std::unique_ptr<geos::geom::Geometry>(geos_mp_->getGeometryN(i)->clone()));
    geoms.push_back(std::move(poly));
    geos_mp_ = factory_->createMultiPolygon(std::move(geoms));
}

template <typename T>
size_t MultiPolygon<T>::num_geometries() const { return geos_mp_->getNumGeometries(); }

template <typename T>
Polygon<double> MultiPolygon<T>::geometry_n(size_t i) const {
    auto* poly = dynamic_cast<const geos::geom::Polygon*>(geos_mp_->getGeometryN(i));
    if (!poly) return Polygon<double>();
    auto* cs = poly->getExteriorRing()->getCoordinatesRO();
    size_t n = cs->getSize();
    std::vector<double> c(n*2);
    for (size_t j = 0; j < n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
    return Polygon<double>(c.data(), n, 2);
}

// -- distance ----------------------------------------------------------------
template <typename T> template <typename U>
double MultiPolygon<T>::distance(const Point<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_mp_.get(), o.geos_point_.get()); return op.distance();
}
template <typename T> template <typename U>
double MultiPolygon<T>::distance(const LineString<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_mp_.get(), o.geos_linestring_.get()); return op.distance();
}
template <typename T> template <typename U>
double MultiPolygon<T>::distance(const Polygon<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_mp_.get(), o.geos_polygon_.get()); return op.distance();
}
template <typename T>
double MultiPolygon<T>::distance(const MultiPolygon& o) const {
    geos::operation::distance::DistanceOp op(geos_mp_.get(), o.geos_mp_.get()); return op.distance();
}
template <typename T> template <typename U>
double MultiPolygon<T>::distance(const MultiLineString<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_mp_.get(), o.geos_mls_.get()); return op.distance();
}

// -- Predicates (macros) ----------------------------------------------------
#define MPY_PRED(PREFIX, OTHER, GETTER, GEOS_FN) \
template <typename T> template <typename U> bool MultiPolygon<T>::PREFIX(const OTHER<U>& o) const { return detail::GEOS_FN(geos_mp_.get(), o.GETTER()); }

MPY_PRED(contains,    Point,      geos_point_.get,      geos_contains)
MPY_PRED(contains,    LineString, geos_linestring_.get,  geos_contains)
MPY_PRED(contains,    Polygon,    geos_polygon_.get,     geos_contains)
MPY_PRED(within,      Point,      geos_point_.get,       geos_within)
MPY_PRED(within,      LineString, geos_linestring_.get,   geos_within)
MPY_PRED(within,      Polygon,    geos_polygon_.get,     geos_within)
MPY_PRED(crosses,     Point,      geos_point_.get,       geos_crosses)
MPY_PRED(crosses,     LineString, geos_linestring_.get,   geos_crosses)
MPY_PRED(crosses,     Polygon,    geos_polygon_.get,     geos_crosses)
MPY_PRED(disjoint,    Point,      geos_point_.get,       geos_disjoint)
MPY_PRED(disjoint,    LineString, geos_linestring_.get,   geos_disjoint)
MPY_PRED(disjoint,    Polygon,    geos_polygon_.get,     geos_disjoint)
MPY_PRED(overlaps,    Point,      geos_point_.get,       geos_overlaps)
MPY_PRED(overlaps,    LineString, geos_linestring_.get,   geos_overlaps)
MPY_PRED(overlaps,    Polygon,    geos_polygon_.get,     geos_overlaps)
MPY_PRED(touches,     Point,      geos_point_.get,       geos_touches)
MPY_PRED(touches,     LineString, geos_linestring_.get,   geos_touches)
MPY_PRED(touches,     Polygon,    geos_polygon_.get,     geos_touches)
MPY_PRED(covers,      Point,      geos_point_.get,       geos_covers)
MPY_PRED(covers,      LineString, geos_linestring_.get,   geos_covers)
MPY_PRED(covers,      Polygon,    geos_polygon_.get,     geos_covers)
MPY_PRED(covered_by,  Point,      geos_point_.get,       geos_covered_by)
MPY_PRED(covered_by,  LineString, geos_linestring_.get,   geos_covered_by)
MPY_PRED(covered_by,  Polygon,    geos_polygon_.get,     geos_covered_by)
MPY_PRED(equals,      Point,      geos_point_.get,       geos_equals)
MPY_PRED(equals,      LineString, geos_linestring_.get,   geos_equals)
MPY_PRED(equals,      Polygon,    geos_polygon_.get,     geos_equals)
#undef MPY_PRED

#define MPY_SELF(METHOD, GEOS_FN) \
template <typename T> bool MultiPolygon<T>::METHOD(const MultiPolygon& o) const { return detail::GEOS_FN(geos_mp_.get(), o.geos_mp_.get()); }
MPY_SELF(contains,    geos_contains)
MPY_SELF(within,      geos_within)
MPY_SELF(disjoint,    geos_disjoint)
MPY_SELF(overlaps,    geos_overlaps)
MPY_SELF(touches,     geos_touches)
MPY_SELF(equals,      geos_equals)
#undef MPY_SELF

template <typename T> template <typename U> bool MultiPolygon<T>::equals_exact(const Point<U>& o, double tol) const { return detail::geos_equals_exact(geos_mp_.get(), o.geos_point_.get(), tol); }
template <typename T> template <typename U> bool MultiPolygon<T>::equals_exact(const LineString<U>& o, double tol) const { return detail::geos_equals_exact(geos_mp_.get(), o.geos_linestring_.get(), tol); }
template <typename T> template <typename U> bool MultiPolygon<T>::equals_exact(const Polygon<U>& o, double tol) const { return detail::geos_equals_exact(geos_mp_.get(), o.geos_polygon_.get(), tol); }
template <typename T> bool MultiPolygon<T>::equals_exact(const MultiPolygon& o, double tol) const { return detail::geos_equals_exact(geos_mp_.get(), o.geos_mp_.get(), tol); }

template <typename T> bool MultiPolygon<T>::intersects(const MultiPolygon& o) const { return geos_mp_->intersects(o.geos_mp_.get()); }
template <typename T> template <typename U> bool MultiPolygon<T>::intersects(const Point<U>& o) const { return geos_mp_->intersects(o.geos_point_.get()); }
template <typename T> template <typename U> bool MultiPolygon<T>::intersects(const LineString<U>& o) const { return geos_mp_->intersects(o.geos_linestring_.get()); }
template <typename T> template <typename U> bool MultiPolygon<T>::intersects(const Polygon<U>& o) const { return geos_mp_->intersects(o.geos_polygon_.get()); }
template <typename T> template <typename U> bool MultiPolygon<T>::intersects(const MultiLineString<U>& o) const { return geos_mp_->intersects(o.geos_mls_.get()); }

// -- relate / relate_pattern -----------------------------------------------
template <typename T> template <typename U> std::string MultiPolygon<T>::relate(const Point<U>& o) const { return detail::geos_relate(geos_mp_.get(), o.geos_point_.get()); }
template <typename T> template <typename U> std::string MultiPolygon<T>::relate(const LineString<U>& o) const { return detail::geos_relate(geos_mp_.get(), o.geos_linestring_.get()); }
template <typename T> template <typename U> std::string MultiPolygon<T>::relate(const Polygon<U>& o) const { return detail::geos_relate(geos_mp_.get(), o.geos_polygon_.get()); }
template <typename T> std::string MultiPolygon<T>::relate(const MultiPolygon& o) const { return detail::geos_relate(geos_mp_.get(), o.geos_mp_.get()); }

template <typename T> template <typename U> bool MultiPolygon<T>::relate_pattern(const Point<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_mp_.get(), o.geos_point_.get(), p); }
template <typename T> template <typename U> bool MultiPolygon<T>::relate_pattern(const LineString<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_mp_.get(), o.geos_linestring_.get(), p); }
template <typename T> template <typename U> bool MultiPolygon<T>::relate_pattern(const Polygon<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_mp_.get(), o.geos_polygon_.get(), p); }
template <typename T> bool MultiPolygon<T>::relate_pattern(const MultiPolygon& o, const std::string& p) const { return detail::geos_relate_pattern(geos_mp_.get(), o.geos_mp_.get(), p); }

// -- hausdorff_distance ----------------------------------------------------
template <typename T> template <typename U> double MultiPolygon<T>::hausdorff_distance(const Point<U>& o) const { return detail::geos_hausdorff_distance(geos_mp_.get(), o.geos_point_.get()); }
template <typename T> template <typename U> double MultiPolygon<T>::hausdorff_distance(const LineString<U>& o) const { return detail::geos_hausdorff_distance(geos_mp_.get(), o.geos_linestring_.get()); }
template <typename T> template <typename U> double MultiPolygon<T>::hausdorff_distance(const Polygon<U>& o) const { return detail::geos_hausdorff_distance(geos_mp_.get(), o.geos_polygon_.get()); }
template <typename T> double MultiPolygon<T>::hausdorff_distance(const MultiPolygon& o) const { return detail::geos_hausdorff_distance(geos_mp_.get(), o.geos_mp_.get()); }

// -- Constructive operations -----------------------------------------------
#define MPY_CONSTRUCT(OP, GEOS_FN) \
template <typename T> MultiPolygon<double> MultiPolygon<T>::OP(const MultiPolygon<double>& o) const { \
    auto res = detail::GEOS_FN(geos_mp_.get(), o.geos_mp_.get()); \
    if (!res || res->isEmpty()) return MultiPolygon<double>(); \
    auto* mp = dynamic_cast<geos::geom::MultiPolygon*>(res.get()); \
    if (mp) { MultiPolygon<double> r; \
        for (size_t i = 0; i < mp->getNumGeometries(); ++i) { \
            auto* poly = dynamic_cast<const geos::geom::Polygon*>(mp->getGeometryN(i)); \
            if (poly) { auto* cs = poly->getExteriorRing()->getCoordinatesRO(); size_t n = cs->getSize(); \
                std::vector<double> c(n*2); for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; } \
                r.add_polygon(c.data(), n, 2); } \
        } return r; } return MultiPolygon<double>(); \
}
MPY_CONSTRUCT(difference,       geos_difference)
MPY_CONSTRUCT(intersection,     geos_intersection)
MPY_CONSTRUCT(union_op,         geos_union)
MPY_CONSTRUCT(symmetric_difference,   geos_sym_difference)
#undef MPY_CONSTRUCT

template <typename T>
MultiPolygon<double> MultiPolygon<T>::simplify(double tol) const {
    auto res = detail::geos_simplify(geos_mp_.get(), tol);
    if (!res || res->isEmpty()) return MultiPolygon<double>();
    auto* mp = dynamic_cast<geos::geom::MultiPolygon*>(res.get());
    if (mp) {
        MultiPolygon<double> r;
        for (size_t i = 0; i < mp->getNumGeometries(); ++i) {
            auto* poly = dynamic_cast<const geos::geom::Polygon*>(mp->getGeometryN(i));
            if (poly) {
                auto* cs = poly->getExteriorRing()->getCoordinatesRO(); size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
                r.add_polygon(c.data(), n, 2);
            }
        }
        return r;
    }
    return MultiPolygon<double>();
}

// -- Accessors ---------------------------------------------------------------
template <typename T> std::string MultiPolygon<T>::wkt() const { return detail::geos_to_wkt(geos_mp_.get()); }
template <typename T> std::string MultiPolygon<T>::wkb_hex() const { return detail::geos_to_wkb_hex(geos_mp_.get()); }
template <typename T> std::string MultiPolygon<T>::type() const { return "MultiPolygon"; }
template <typename T> std::string MultiPolygon<T>::geom_type() const { return detail::geos_geom_type(geos_mp_.get()); }
template <typename T> bool MultiPolygon<T>::has_z() const { return detail::geos_has_z(geos_mp_.get()); }

// -- Properties ---------------------------------------------------------------
template <typename T> bool MultiPolygon<T>::is_empty() const { return detail::geos_is_empty(geos_mp_.get()); }
template <typename T> bool MultiPolygon<T>::is_simple() const { return detail::geos_is_simple(geos_mp_.get()); }
template <typename T> bool MultiPolygon<T>::is_valid() const { return detail::geos_is_valid(geos_mp_.get()); }
template <typename T> double MultiPolygon<T>::area() const { return geos_mp_->getArea(); }
template <typename T> double MultiPolygon<T>::length() const { return geos_mp_->getLength(); }
template <typename T> std::vector<double> MultiPolygon<T>::bounds() const { return detail::geos_bounds(geos_mp_.get()); }

// -- Topology ------------------------------------------------------------------
template <typename T>
Point<double> MultiPolygon<T>::centroid() const {
    auto c = geos_mp_->getCentroid();
    if (!c) return Point<double>(0, 0);
    auto* coord = c->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

template <typename T>
MultiPolygon<double> MultiPolygon<T>::convex_hull() const {
    auto res = detail::geos_convex_hull(geos_mp_.get());
    if (!res || res->isEmpty()) return MultiPolygon<double>();
    auto* mp = dynamic_cast<geos::geom::MultiPolygon*>(res.get());
    if (mp) {
        MultiPolygon<double> r;
        for (size_t i = 0; i < mp->getNumGeometries(); ++i) {
            auto* poly = dynamic_cast<const geos::geom::Polygon*>(mp->getGeometryN(i));
            if (poly) {
                auto* cs = poly->getExteriorRing()->getCoordinatesRO(); size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
                r.add_polygon(c.data(), n, 2);
            }
        }
        return r;
    }
    return MultiPolygon<double>();
}

template <typename T>
MultiPolygon<double> MultiPolygon<T>::buffer(double distance) const {
    auto buf = geos_mp_->buffer(distance, 16);
    if (!buf || buf->isEmpty()) return MultiPolygon<double>();
    auto* mp = dynamic_cast<geos::geom::MultiPolygon*>(buf.get());
    if (mp) {
        MultiPolygon<double> r;
        for (size_t i = 0; i < mp->getNumGeometries(); ++i) {
            auto* poly = dynamic_cast<const geos::geom::Polygon*>(mp->getGeometryN(i));
            if (poly) {
                auto* cs = poly->getExteriorRing()->getCoordinatesRO(); size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
                r.add_polygon(c.data(), n, 2);
            }
        }
        return r;
    }
    return MultiPolygon<double>();
}

template <typename T>
void MultiPolygon<T>::normalize() { geos_mp_->normalize(); }

} // namespace geometry
} // namespace shapely
