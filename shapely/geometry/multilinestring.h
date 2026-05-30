// Python Source: shapely/geometry/multilinestring.py
// Line Range: L10-L70 (class MultiLineString + MultiLineStringAdapter)
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
#include <geos/geom/MultiLineString.h>
#include <geos/geom/LineString.h>

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_LINESTRING_DEFINED
template <typename T> class LineString;
#endif
#ifndef SHAPELY_GEOMETRY_POINT_DEFINED
template <typename T> class Point;
#endif
#ifndef SHAPELY_GEOMETRY_POLYGON_DEFINED
template <typename T> class Polygon;
#endif
#ifndef SHAPELY_GEOMETRY_MULTIPOLYGON_DEFINED
template <typename T> class MultiPolygon;
#endif

template <typename T = double>
class MultiLineString {
public:
    MultiLineString();

    /// Add a line from raw coordinates
    void add_line(const T* coords, size_t rows, size_t cols = 2);

    MultiLineString(MultiLineString&&) = default;
    MultiLineString& operator=(MultiLineString&&) = default;

    // -- Access to individual lines --
    size_t num_geometries() const;
    LineString<double> geometry_n(size_t i) const;

    // -- Distance --
    template <typename U> double distance(const Point<U>& other) const;
    template <typename U> double distance(const LineString<U>& other) const;
    template <typename U> double distance(const Polygon<U>& other) const;
    template <typename U> double distance(const MultiPolygon<U>& other) const;
    double distance(const MultiLineString& other) const;

    // -- Predicates --
    template <typename U> bool contains(const Point<U>& other) const;
    template <typename U> bool contains(const LineString<U>& other) const;
    bool contains(const MultiLineString& other) const;
    template <typename U> bool within(const Point<U>& other) const;
    template <typename U> bool within(const LineString<U>& other) const;
    bool within(const MultiLineString& other) const;
    template <typename U> bool crosses(const Point<U>& other) const;
    template <typename U> bool crosses(const LineString<U>& other) const;
    bool crosses(const MultiLineString& other) const;
    template <typename U> bool disjoint(const Point<U>& other) const;
    template <typename U> bool disjoint(const LineString<U>& other) const;
    bool disjoint(const MultiLineString& other) const;
    template <typename U> bool overlaps(const Point<U>& other) const;
    template <typename U> bool overlaps(const LineString<U>& other) const;
    bool overlaps(const MultiLineString& other) const;
    template <typename U> bool touches(const Point<U>& other) const;
    template <typename U> bool touches(const LineString<U>& other) const;
    bool touches(const MultiLineString& other) const;
    template <typename U> bool covers(const Point<U>& other) const;
    template <typename U> bool covers(const LineString<U>& other) const;
    template <typename U> bool covered_by(const Point<U>& other) const;
    template <typename U> bool covered_by(const LineString<U>& other) const;
    template <typename U> bool equals(const Point<U>& other) const;
    template <typename U> bool equals(const LineString<U>& other) const;
    bool equals(const MultiLineString& other) const;
    template <typename U> bool equals_exact(const Point<U>& other, double tol) const;
    template <typename U> bool equals_exact(const LineString<U>& other, double tol) const;
    bool equals_exact(const MultiLineString& other, double tol) const;

    bool intersects(const MultiLineString& other) const;
    template <typename U> bool intersects(const Point<U>& other) const;
    template <typename U> bool intersects(const LineString<U>& other) const;

    // -- DE-9IM --
    template <typename U> std::string relate(const Point<U>& other) const;
    template <typename U> std::string relate(const LineString<U>& other) const;
    std::string relate(const MultiLineString& other) const;
    template <typename U> bool relate_pattern(const Point<U>& other, const std::string& p) const;
    template <typename U> bool relate_pattern(const LineString<U>& other, const std::string& p) const;
    bool relate_pattern(const MultiLineString& other, const std::string& p) const;

    // -- Hausdorff distance --
    template <typename U> double hausdorff_distance(const Point<U>& other) const;
    template <typename U> double hausdorff_distance(const LineString<U>& other) const;
    double hausdorff_distance(const MultiLineString& other) const;

    // -- Constructive operations --
    MultiLineString<double> difference(const MultiLineString<double>& other) const;
    MultiLineString<double> intersection(const MultiLineString<double>& other) const;
    MultiLineString<double> union_op(const MultiLineString<double>& other) const;
    MultiLineString<double> symmetric_difference(const MultiLineString<double>& other) const;
    MultiLineString<double> simplify(double tolerance) const;

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
    bool is_closed() const;
    double area() const;
    double length() const;
    std::vector<double> bounds() const;

    // -- Topology --
    Point<double> centroid() const;
    MultiLineString<double> convex_hull() const;
    MultiLineString<double> buffer(double distance) const;
    void normalize();

private:
    template <typename U> friend class Point;
    template <typename U> friend class LineString;
    template <typename U> friend class Polygon;
    template <typename U> friend class MultiPolygon;
    std::unique_ptr<geos::geom::MultiLineString> geos_mls_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_MULTILINESTRING_DEFINED

// ============================================================================
// Implementation
// ============================================================================

#include "shapely/geometry/point.h"
#include "shapely/geometry/linestring.h"
#include "shapely/geometry/polygon.h"
#include "shapely/geometry/multipolygon.h"
#include "shapely/geometry/base.h"
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/operation/distance/DistanceOp.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

template <typename T>
MultiLineString<T>::MultiLineString() {
    factory_ = geos::geom::GeometryFactory::create();
    geos_mls_ = factory_->createMultiLineString();
}

template <typename T>
void MultiLineString<T>::add_line(const T* coords, size_t rows, size_t cols) {
    if (cols < 2 || rows < 2) return;
    auto cs = factory_->getCoordinateSequenceFactory()->create(rows, 2);
    for (size_t i = 0; i < rows; ++i)
        cs->setAt(geos::geom::Coordinate(static_cast<double>(coords[i*cols]),
                                          static_cast<double>(coords[i*cols+1])), i);
    auto ls = factory_->createLineString(std::move(cs));
    std::vector<std::unique_ptr<geos::geom::Geometry>> geoms;
    for (size_t i = 0; i < geos_mls_->getNumGeometries(); ++i)
        geoms.push_back(std::unique_ptr<geos::geom::Geometry>(geos_mls_->getGeometryN(i)->clone()));
    geoms.push_back(std::move(ls));
    geos_mls_ = factory_->createMultiLineString(std::move(geoms));
}

template <typename T>
size_t MultiLineString<T>::num_geometries() const { return geos_mls_->getNumGeometries(); }

template <typename T>
LineString<double> MultiLineString<T>::geometry_n(size_t i) const {
    auto* ls = dynamic_cast<const geos::geom::LineString*>(geos_mls_->getGeometryN(i));
    if (!ls) return LineString<double>(nullptr, 0, 0);
    auto* cs = ls->getCoordinatesRO();
    size_t n = cs->getSize();
    std::vector<double> c(n*2);
    for (size_t j = 0; j < n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
    return LineString<double>(c.data(), n, 2);
}

// -- distance ----------------------------------------------------------------
template <typename T> template <typename U>
double MultiLineString<T>::distance(const Point<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_mls_.get(), o.geos_point_.get()); return op.distance();
}
template <typename T> template <typename U>
double MultiLineString<T>::distance(const LineString<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_mls_.get(), o.geos_linestring_.get()); return op.distance();
}
template <typename T>
double MultiLineString<T>::distance(const MultiLineString& o) const {
    geos::operation::distance::DistanceOp op(geos_mls_.get(), o.geos_mls_.get()); return op.distance();
}
template <typename T> template <typename U>
double MultiLineString<T>::distance(const Polygon<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_mls_.get(), o.geos_polygon_.get()); return op.distance();
}
template <typename T> template <typename U>
double MultiLineString<T>::distance(const MultiPolygon<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_mls_.get(), o.geos_mp_.get()); return op.distance();
}

// -- Predicates (macros) ----------------------------------------------------
#define MLS_PRED_PT(METHOD, GEOS_FN) \
template <typename T> template <typename U> bool MultiLineString<T>::METHOD(const Point<U>& o) const { return detail::GEOS_FN(geos_mls_.get(), o.geos_point_.get()); }
MLS_PRED_PT(contains,   geos_contains)
MLS_PRED_PT(within,     geos_within)
MLS_PRED_PT(crosses,    geos_crosses)
MLS_PRED_PT(disjoint,   geos_disjoint)
MLS_PRED_PT(overlaps,   geos_overlaps)
MLS_PRED_PT(touches,    geos_touches)
MLS_PRED_PT(covers,     geos_covers)
MLS_PRED_PT(covered_by, geos_covered_by)
MLS_PRED_PT(equals,     geos_equals)
#undef MLS_PRED_PT

#define MLS_PRED_LS(METHOD, GEOS_FN) \
template <typename T> template <typename U> bool MultiLineString<T>::METHOD(const LineString<U>& o) const { return detail::GEOS_FN(geos_mls_.get(), o.geos_linestring_.get()); }
MLS_PRED_LS(contains,   geos_contains)
MLS_PRED_LS(within,     geos_within)
MLS_PRED_LS(crosses,    geos_crosses)
MLS_PRED_LS(disjoint,   geos_disjoint)
MLS_PRED_LS(overlaps,   geos_overlaps)
MLS_PRED_LS(touches,    geos_touches)
MLS_PRED_LS(covers,     geos_covers)
MLS_PRED_LS(covered_by, geos_covered_by)
MLS_PRED_LS(equals,     geos_equals)
#undef MLS_PRED_LS

#define MLS_PRED_SELF(METHOD, GEOS_FN) \
template <typename T> bool MultiLineString<T>::METHOD(const MultiLineString& o) const { return detail::GEOS_FN(geos_mls_.get(), o.geos_mls_.get()); }
MLS_PRED_SELF(contains,    geos_contains)
MLS_PRED_SELF(within,      geos_within)
MLS_PRED_SELF(crosses,     geos_crosses)
MLS_PRED_SELF(disjoint,    geos_disjoint)
MLS_PRED_SELF(overlaps,    geos_overlaps)
MLS_PRED_SELF(touches,     geos_touches)
MLS_PRED_SELF(equals,      geos_equals)
#undef MLS_PRED_SELF

template <typename T> template <typename U>
bool MultiLineString<T>::equals_exact(const Point<U>& o, double tol) const { return detail::geos_equals_exact(geos_mls_.get(), o.geos_point_.get(), tol); }
template <typename T> template <typename U>
bool MultiLineString<T>::equals_exact(const LineString<U>& o, double tol) const { return detail::geos_equals_exact(geos_mls_.get(), o.geos_linestring_.get(), tol); }
template <typename T>
bool MultiLineString<T>::equals_exact(const MultiLineString& o, double tol) const { return detail::geos_equals_exact(geos_mls_.get(), o.geos_mls_.get(), tol); }

template <typename T>
bool MultiLineString<T>::intersects(const MultiLineString& o) const { return geos_mls_->intersects(o.geos_mls_.get()); }
template <typename T> template <typename U>
bool MultiLineString<T>::intersects(const Point<U>& o) const { return geos_mls_->intersects(o.geos_point_.get()); }
template <typename T> template <typename U>
bool MultiLineString<T>::intersects(const LineString<U>& o) const { return geos_mls_->intersects(o.geos_linestring_.get()); }

// -- relate / relate_pattern -----------------------------------------------
template <typename T> template <typename U> std::string MultiLineString<T>::relate(const Point<U>& o) const { return detail::geos_relate(geos_mls_.get(), o.geos_point_.get()); }
template <typename T> template <typename U> std::string MultiLineString<T>::relate(const LineString<U>& o) const { return detail::geos_relate(geos_mls_.get(), o.geos_linestring_.get()); }
template <typename T> std::string MultiLineString<T>::relate(const MultiLineString& o) const { return detail::geos_relate(geos_mls_.get(), o.geos_mls_.get()); }

template <typename T> template <typename U> bool MultiLineString<T>::relate_pattern(const Point<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_mls_.get(), o.geos_point_.get(), p); }
template <typename T> template <typename U> bool MultiLineString<T>::relate_pattern(const LineString<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_mls_.get(), o.geos_linestring_.get(), p); }
template <typename T> bool MultiLineString<T>::relate_pattern(const MultiLineString& o, const std::string& p) const { return detail::geos_relate_pattern(geos_mls_.get(), o.geos_mls_.get(), p); }

// -- hausdorff_distance ----------------------------------------------------
template <typename T> template <typename U> double MultiLineString<T>::hausdorff_distance(const Point<U>& o) const { return detail::geos_hausdorff_distance(geos_mls_.get(), o.geos_point_.get()); }
template <typename T> template <typename U> double MultiLineString<T>::hausdorff_distance(const LineString<U>& o) const { return detail::geos_hausdorff_distance(geos_mls_.get(), o.geos_linestring_.get()); }
template <typename T> double MultiLineString<T>::hausdorff_distance(const MultiLineString& o) const { return detail::geos_hausdorff_distance(geos_mls_.get(), o.geos_mls_.get()); }

// -- Constructive operations -----------------------------------------------
template <typename T>
MultiLineString<double> MultiLineString<T>::difference(const MultiLineString<double>& o) const {
    auto res = detail::geos_difference(geos_mls_.get(), o.geos_mls_.get());
    if (!res || res->isEmpty()) return MultiLineString<double>();
    auto* mls = dynamic_cast<geos::geom::MultiLineString*>(res.get());
    if (mls) {
        MultiLineString<double> result;
        for (size_t i = 0; i < mls->getNumGeometries(); ++i) {
            auto* ls = dynamic_cast<const geos::geom::LineString*>(mls->getGeometryN(i));
            if (ls) {
                auto* cs = ls->getCoordinatesRO();
                size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
                result.add_line(c.data(), n, 2);
            }
        }
        return result;
    }
    return MultiLineString<double>();
}
template <typename T>
MultiLineString<double> MultiLineString<T>::intersection(const MultiLineString<double>& o) const {
    auto res = detail::geos_intersection(geos_mls_.get(), o.geos_mls_.get());
    if (!res || res->isEmpty()) return MultiLineString<double>();
    auto* mls = dynamic_cast<geos::geom::MultiLineString*>(res.get());
    if (mls) {
        MultiLineString<double> result;
        for (size_t i = 0; i < mls->getNumGeometries(); ++i) {
            auto* ls = dynamic_cast<const geos::geom::LineString*>(mls->getGeometryN(i));
            if (ls) {
                auto* cs = ls->getCoordinatesRO();
                size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
                result.add_line(c.data(), n, 2);
            }
        }
        return result;
    }
    return MultiLineString<double>();
}
template <typename T>
MultiLineString<double> MultiLineString<T>::union_op(const MultiLineString<double>& o) const {
    auto res = detail::geos_union(geos_mls_.get(), o.geos_mls_.get());
    if (!res || res->isEmpty()) return MultiLineString<double>();
    auto* mls = dynamic_cast<geos::geom::MultiLineString*>(res.get());
    if (mls) {
        MultiLineString<double> result;
        for (size_t i = 0; i < mls->getNumGeometries(); ++i) {
            auto* ls = dynamic_cast<const geos::geom::LineString*>(mls->getGeometryN(i));
            if (ls) {
                auto* cs = ls->getCoordinatesRO();
                size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
                result.add_line(c.data(), n, 2);
            }
        }
        return result;
    }
    return MultiLineString<double>();
}
template <typename T>
MultiLineString<double> MultiLineString<T>::symmetric_difference(const MultiLineString<double>& o) const {
    auto res = detail::geos_sym_difference(geos_mls_.get(), o.geos_mls_.get());
    if (!res || res->isEmpty()) return MultiLineString<double>();
    auto* mls = dynamic_cast<geos::geom::MultiLineString*>(res.get());
    if (mls) {
        MultiLineString<double> result;
        for (size_t i = 0; i < mls->getNumGeometries(); ++i) {
            auto* ls = dynamic_cast<const geos::geom::LineString*>(mls->getGeometryN(i));
            if (ls) {
                auto* cs = ls->getCoordinatesRO();
                size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
                result.add_line(c.data(), n, 2);
            }
        }
        return result;
    }
    return MultiLineString<double>();
}
template <typename T>
MultiLineString<double> MultiLineString<T>::simplify(double tol) const {
    auto res = detail::geos_simplify(geos_mls_.get(), tol);
    if (!res || res->isEmpty()) return MultiLineString<double>();
    auto* mls = dynamic_cast<geos::geom::MultiLineString*>(res.get());
    if (mls) {
        MultiLineString<double> result;
        for (size_t i = 0; i < mls->getNumGeometries(); ++i) {
            auto* ls = dynamic_cast<const geos::geom::LineString*>(mls->getGeometryN(i));
            if (ls) {
                auto* cs = ls->getCoordinatesRO();
                size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
                result.add_line(c.data(), n, 2);
            }
        }
        return result;
    }
    return MultiLineString<double>();
}

// -- Accessors ---------------------------------------------------------------
template <typename T> std::string MultiLineString<T>::wkt() const { return detail::geos_to_wkt(geos_mls_.get()); }
template <typename T> std::string MultiLineString<T>::wkb_hex() const { return detail::geos_to_wkb_hex(geos_mls_.get()); }
template <typename T> std::string MultiLineString<T>::type() const { return "MultiLineString"; }
template <typename T> std::string MultiLineString<T>::geom_type() const { return detail::geos_geom_type(geos_mls_.get()); }
template <typename T> bool MultiLineString<T>::has_z() const { return detail::geos_has_z(geos_mls_.get()); }

// -- Properties ---------------------------------------------------------------
template <typename T> bool MultiLineString<T>::is_empty() const { return detail::geos_is_empty(geos_mls_.get()); }
template <typename T> bool MultiLineString<T>::is_simple() const { return detail::geos_is_simple(geos_mls_.get()); }
template <typename T> bool MultiLineString<T>::is_valid() const { return detail::geos_is_valid(geos_mls_.get()); }
template <typename T> bool MultiLineString<T>::is_closed() const { return geos_mls_->isClosed(); }
template <typename T> double MultiLineString<T>::area() const { return 0.0; }
template <typename T> double MultiLineString<T>::length() const { return geos_mls_->getLength(); }
template <typename T> std::vector<double> MultiLineString<T>::bounds() const { return detail::geos_bounds(geos_mls_.get()); }

// -- Topology ------------------------------------------------------------------
template <typename T>
Point<double> MultiLineString<T>::centroid() const {
    auto c = geos_mls_->getCentroid();
    if (!c) return Point<double>(0, 0);
    auto* coord = c->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

template <typename T>
MultiLineString<double> MultiLineString<T>::convex_hull() const {
    auto res = detail::geos_convex_hull(geos_mls_.get());
    if (!res || res->isEmpty()) return MultiLineString<double>();
    auto* mls = dynamic_cast<geos::geom::MultiLineString*>(res.get());
    if (mls) {
        MultiLineString<double> result;
        for (size_t i = 0; i < mls->getNumGeometries(); ++i) {
            auto* ls = dynamic_cast<const geos::geom::LineString*>(mls->getGeometryN(i));
            if (ls) {
                auto* cs = ls->getCoordinatesRO(); size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
                result.add_line(c.data(), n, 2);
            }
        }
        return result;
    }
    return MultiLineString<double>();
}

template <typename T>
MultiLineString<double> MultiLineString<T>::buffer(double distance) const {
    auto buf = geos_mls_->buffer(distance, 16);
    if (!buf || buf->isEmpty()) return MultiLineString<double>();
    auto* mls = dynamic_cast<geos::geom::MultiLineString*>(buf.get());
    if (mls) {
        MultiLineString<double> result;
        for (size_t i = 0; i < mls->getNumGeometries(); ++i) {
            auto* ls = dynamic_cast<const geos::geom::LineString*>(mls->getGeometryN(i));
            if (ls) {
                auto* cs = ls->getCoordinatesRO(); size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t j=0; j<n; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
                result.add_line(c.data(), n, 2);
            }
        }
        return result;
    }
    return MultiLineString<double>();
}

template <typename T>
void MultiLineString<T>::normalize() { geos_mls_->normalize(); }

} // namespace geometry
} // namespace shapely
