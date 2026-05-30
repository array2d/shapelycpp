// Python Source: shapely/geometry/multipoint.py
// Line Range: L10-L60 (class MultiPoint + MultiPointAdapter)
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
#include <geos/geom/MultiPoint.h>
#include <geos/geom/Point.h>

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_POINT_DEFINED
template <typename T> class Point;
#endif

template <typename T = double>
class MultiPoint {
public:
    MultiPoint();
    MultiPoint(const T* coords, size_t n_pts, size_t dims = 2);
    MultiPoint(MultiPoint&&) = default;
    MultiPoint& operator=(MultiPoint&&) = default;

    // -- Access to individual points --
    size_t num_geometries() const;
    Point<double> geometry_n(size_t i) const;

    // -- Raw coordinate access --
    const T* data() const { return coords_.data(); }
    size_t rows() const { return n_pts_; }
    size_t cols() const { return dims_; }

    // -- coords, xy (Python: .coords, .xy) --
    std::vector<std::tuple<T, T>> coords() const;
    std::tuple<std::vector<T>, std::vector<T>> xy() const;

    // -- Distance --
    template <typename U> double distance(const Point<U>& other) const;
    template <typename U> double distance(const MultiPoint<U>& other) const;

    // -- Predicates --
    template <typename U> bool contains(const Point<U>& other) const;
    template <typename U> bool contains(const MultiPoint<U>& other) const;
    template <typename U> bool within(const Point<U>& other) const;
    template <typename U> bool within(const MultiPoint<U>& other) const;
    template <typename U> bool crosses(const Point<U>& other) const;
    bool crosses(const MultiPoint& other) const;
    bool disjoint(const MultiPoint& other) const;
    template <typename U> bool disjoint(const Point<U>& other) const;
    template <typename U> bool overlaps(const Point<U>& other) const;
    bool overlaps(const MultiPoint& other) const;
    template <typename U> bool touches(const Point<U>& other) const;
    bool touches(const MultiPoint& other) const;
    template <typename U> bool covers(const Point<U>& other) const;
    template <typename U> bool covered_by(const Point<U>& other) const;
    template <typename U> bool equals(const Point<U>& other) const;
    bool equals(const MultiPoint& other) const;
    template <typename U> bool equals_exact(const Point<U>& other, double tol) const;
    bool equals_exact(const MultiPoint& other, double tol) const;

    bool intersects(const MultiPoint& other) const;
    template <typename U> bool intersects(const Point<U>& other) const;

    // -- DE-9IM --
    template <typename U> std::string relate(const Point<U>& other) const;
    std::string relate(const MultiPoint& other) const;
    template <typename U> bool relate_pattern(const Point<U>& other, const std::string& p) const;
    bool relate_pattern(const MultiPoint& other, const std::string& p) const;

    // -- Hausdorff distance --
    template <typename U> double hausdorff_distance(const Point<U>& other) const;
    double hausdorff_distance(const MultiPoint& other) const;

    // -- Constructive operations --
    MultiPoint<double> difference(const MultiPoint<double>& other) const;
    template <typename U> MultiPoint<double> difference(const Point<U>& other) const;
    MultiPoint<double> intersection(const MultiPoint<double>& other) const;
    template <typename U> Point<double> intersection(const Point<U>& other) const;
    MultiPoint<double> union_op(const MultiPoint<double>& other) const;
    template <typename U> MultiPoint<double> union_op(const Point<U>& other) const;
    MultiPoint<double> symmetric_difference(const MultiPoint<double>& other) const;
    template <typename U> MultiPoint<double> symmetric_difference(const Point<U>& other) const;
    MultiPoint<double> simplify(double tolerance) const;

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
    MultiPoint<double> convex_hull() const;
    MultiPoint<double> envelope() const;
    MultiPoint<double> representative_point() const;
    MultiPoint<double> buffer(double distance) const;
    void normalize();

private:
    template <typename U> friend class Point;
    template <typename U> friend class LineString;
    template <typename U> friend class Polygon;
    std::vector<T> coords_;
    size_t n_pts_ = 0, dims_ = 0;
    std::unique_ptr<geos::geom::MultiPoint> geos_multipoint_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_MULTIPOINT_DEFINED

// ============================================================================
// Implementation
// ============================================================================

#include "shapely/geometry/point.h"
#include "shapely/geometry/base.h"
#include <geos/geom/Point.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/operation/distance/DistanceOp.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

// Python: shapely/geometry/multipoint.py::__init__:L16
template <typename T>
MultiPoint<T>::MultiPoint() : n_pts_(0), dims_(0) {
    factory_ = geos::geom::GeometryFactory::create();
    geos_multipoint_ = factory_->createMultiPoint();
}

template <typename T>
MultiPoint<T>::MultiPoint(const T* coords, size_t n_pts, size_t dims)
    : coords_(coords, coords + n_pts * dims), n_pts_(n_pts), dims_(dims)
{
    if (dims < 2) throw std::runtime_error("MultiPoint: dims must be >= 2");
    factory_ = geos::geom::GeometryFactory::create();
    std::vector<std::unique_ptr<geos::geom::Geometry>> pts;
    for (size_t i = 0; i < n_pts; ++i) {
        pts.emplace_back(factory_->createPoint(geos::geom::Coordinate(
            static_cast<double>(coords_[i*dims]), static_cast<double>(coords_[i*dims+1]))));
    }
    geos_multipoint_ = factory_->createMultiPoint(std::move(pts));
}

template <typename T>
size_t MultiPoint<T>::num_geometries() const { return geos_multipoint_->getNumGeometries(); }

template <typename T>
Point<double> MultiPoint<T>::geometry_n(size_t i) const {
    auto* pt = dynamic_cast<const geos::geom::Point*>(geos_multipoint_->getGeometryN(i));
    if (!pt) return Point<double>(0, 0);
    auto* coord = pt->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

// -- coords, xy -------------------------------------------------------------
template <typename T>
std::vector<std::tuple<T, T>> MultiPoint<T>::coords() const {
    std::vector<std::tuple<T, T>> r; r.reserve(n_pts_);
    for (size_t i = 0; i < n_pts_; ++i) r.emplace_back(coords_[i*dims_], coords_[i*dims_+1]);
    return r;
}

template <typename T>
std::tuple<std::vector<T>, std::vector<T>> MultiPoint<T>::xy() const {
    std::vector<T> xs(n_pts_), ys(n_pts_);
    for (size_t i = 0; i < n_pts_; ++i) { xs[i]=coords_[i*dims_]; ys[i]=coords_[i*dims_+1]; }
    return {xs, ys};
}

// -- distance ----------------------------------------------------------------
template <typename T> template <typename U>
double MultiPoint<T>::distance(const Point<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_multipoint_.get(), o.geos_point_.get());
    return op.distance();
}
template <typename T> template <typename U>
double MultiPoint<T>::distance(const MultiPoint<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_multipoint_.get(), o.geos_multipoint_.get());
    return op.distance();
}

// -- Predicates ------------------------------------------------------------
#define MP_PRED_PT(METHOD, GEOS_FN) \
template <typename T> template <typename U> bool MultiPoint<T>::METHOD(const Point<U>& o) const { return detail::GEOS_FN(geos_multipoint_.get(), o.geos_point_.get()); }
MP_PRED_PT(contains,   geos_contains)
MP_PRED_PT(within,     geos_within)
MP_PRED_PT(crosses,    geos_crosses)
MP_PRED_PT(disjoint,   geos_disjoint)
MP_PRED_PT(overlaps,   geos_overlaps)
MP_PRED_PT(touches,    geos_touches)
MP_PRED_PT(covers,     geos_covers)
MP_PRED_PT(covered_by, geos_covered_by)
MP_PRED_PT(equals,     geos_equals)
#undef MP_PRED_PT

template <typename T> template <typename U>
bool MultiPoint<T>::contains(const MultiPoint<U>& o) const { return detail::geos_contains(geos_multipoint_.get(), o.geos_multipoint_.get()); }
template <typename T> template <typename U>
bool MultiPoint<T>::within(const MultiPoint<U>& o) const { return detail::geos_within(geos_multipoint_.get(), o.geos_multipoint_.get()); }
template <typename T>
bool MultiPoint<T>::crosses(const MultiPoint& o) const { return detail::geos_crosses(geos_multipoint_.get(), o.geos_multipoint_.get()); }
template <typename T>
bool MultiPoint<T>::disjoint(const MultiPoint& o) const { return detail::geos_disjoint(geos_multipoint_.get(), o.geos_multipoint_.get()); }
template <typename T>
bool MultiPoint<T>::overlaps(const MultiPoint& o) const { return detail::geos_overlaps(geos_multipoint_.get(), o.geos_multipoint_.get()); }
template <typename T>
bool MultiPoint<T>::touches(const MultiPoint& o) const { return detail::geos_touches(geos_multipoint_.get(), o.geos_multipoint_.get()); }
template <typename T>
bool MultiPoint<T>::equals(const MultiPoint& o) const { return detail::geos_equals(geos_multipoint_.get(), o.geos_multipoint_.get()); }
template <typename T>
bool MultiPoint<T>::equals_exact(const MultiPoint& o, double tol) const { return detail::geos_equals_exact(geos_multipoint_.get(), o.geos_multipoint_.get(), tol); }
template <typename T> template <typename U>
bool MultiPoint<T>::equals_exact(const Point<U>& o, double tol) const { return detail::geos_equals_exact(geos_multipoint_.get(), o.geos_point_.get(), tol); }

template <typename T>
bool MultiPoint<T>::intersects(const MultiPoint& o) const { return geos_multipoint_->intersects(o.geos_multipoint_.get()); }
template <typename T> template <typename U>
bool MultiPoint<T>::intersects(const Point<U>& o) const { return geos_multipoint_->intersects(o.geos_point_.get()); }

// -- relate / relate_pattern -----------------------------------------------
template <typename T> template <typename U>
std::string MultiPoint<T>::relate(const Point<U>& o) const { return detail::geos_relate(geos_multipoint_.get(), o.geos_point_.get()); }
template <typename T>
std::string MultiPoint<T>::relate(const MultiPoint& o) const { return detail::geos_relate(geos_multipoint_.get(), o.geos_multipoint_.get()); }
template <typename T> template <typename U>
bool MultiPoint<T>::relate_pattern(const Point<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_multipoint_.get(), o.geos_point_.get(), p); }
template <typename T>
bool MultiPoint<T>::relate_pattern(const MultiPoint& o, const std::string& p) const { return detail::geos_relate_pattern(geos_multipoint_.get(), o.geos_multipoint_.get(), p); }

// -- hausdorff_distance ----------------------------------------------------
template <typename T> template <typename U>
double MultiPoint<T>::hausdorff_distance(const Point<U>& o) const { return detail::geos_hausdorff_distance(geos_multipoint_.get(), o.geos_point_.get()); }
template <typename T>
double MultiPoint<T>::hausdorff_distance(const MultiPoint& o) const { return detail::geos_hausdorff_distance(geos_multipoint_.get(), o.geos_multipoint_.get()); }

// -- Constructive operations -----------------------------------------------
template <typename T>
MultiPoint<double> MultiPoint<T>::difference(const MultiPoint<double>& o) const {
    auto res = detail::geos_difference(geos_multipoint_.get(), o.geos_multipoint_.get());
    if (!res || res->isEmpty()) return MultiPoint<double>();
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(res.get());
    if (!mpt) return MultiPoint<double>();
    size_t n = mpt->getNumGeometries();
    std::vector<double> c(n * 2);
    for (size_t i = 0; i < n; ++i) {
        auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(i));
        if (pt) { c[i*2]=pt->getX(); c[i*2+1]=pt->getY(); }
    }
    return MultiPoint<double>(c.data(), n, 2);
}
template <typename T> template <typename U>
MultiPoint<double> MultiPoint<T>::difference(const Point<U>& o) const {
    auto res = detail::geos_difference(geos_multipoint_.get(), o.geos_point_.get());
    if (!res || res->isEmpty()) return MultiPoint<double>();
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(res.get());
    if (!mpt) return MultiPoint<double>();
    size_t n = mpt->getNumGeometries();
    std::vector<double> c(n * 2);
    for (size_t i = 0; i < n; ++i) {
        auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(i));
        if (pt) { c[i*2]=pt->getX(); c[i*2+1]=pt->getY(); }
    }
    return MultiPoint<double>(c.data(), n, 2);
}
template <typename T>
MultiPoint<double> MultiPoint<T>::intersection(const MultiPoint<double>& o) const {
    auto res = detail::geos_intersection(geos_multipoint_.get(), o.geos_multipoint_.get());
    if (!res || res->isEmpty()) return MultiPoint<double>();
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(res.get());
    if (!mpt) return MultiPoint<double>();
    size_t n = mpt->getNumGeometries();
    std::vector<double> c(n * 2);
    for (size_t i = 0; i < n; ++i) {
        auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(i));
        if (pt) { c[i*2]=pt->getX(); c[i*2+1]=pt->getY(); }
    }
    return MultiPoint<double>(c.data(), n, 2);
}
template <typename T> template <typename U>
Point<double> MultiPoint<T>::intersection(const Point<U>& o) const {
    auto res = detail::geos_intersection(geos_multipoint_.get(), o.geos_point_.get());
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* pt = dynamic_cast<const geos::geom::Point*>(res.get());
    if (!pt) return Point<double>(0, 0);
    return Point<double>(pt->getX(), pt->getY());
}
template <typename T>
MultiPoint<double> MultiPoint<T>::union_op(const MultiPoint<double>& o) const {
    auto res = detail::geos_union(geos_multipoint_.get(), o.geos_multipoint_.get());
    if (!res || res->isEmpty()) return MultiPoint<double>();
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(res.get());
    if (!mpt) return MultiPoint<double>();
    size_t n = mpt->getNumGeometries();
    std::vector<double> c(n * 2);
    for (size_t i = 0; i < n; ++i) {
        auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(i));
        if (pt) { c[i*2]=pt->getX(); c[i*2+1]=pt->getY(); }
    }
    return MultiPoint<double>(c.data(), n, 2);
}
template <typename T> template <typename U>
MultiPoint<double> MultiPoint<T>::union_op(const Point<U>& o) const {
    auto res = detail::geos_union(geos_multipoint_.get(), o.geos_point_.get());
    if (!res || res->isEmpty()) return MultiPoint<double>();
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(res.get());
    if (!mpt) return MultiPoint<double>();
    size_t n = mpt->getNumGeometries();
    std::vector<double> c(n * 2);
    for (size_t i = 0; i < n; ++i) {
        auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(i));
        if (pt) { c[i*2]=pt->getX(); c[i*2+1]=pt->getY(); }
    }
    return MultiPoint<double>(c.data(), n, 2);
}
template <typename T>
MultiPoint<double> MultiPoint<T>::symmetric_difference(const MultiPoint<double>& o) const {
    auto res = detail::geos_sym_difference(geos_multipoint_.get(), o.geos_multipoint_.get());
    if (!res || res->isEmpty()) return MultiPoint<double>();
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(res.get());
    if (!mpt) return MultiPoint<double>();
    size_t n = mpt->getNumGeometries();
    std::vector<double> c(n * 2);
    for (size_t i = 0; i < n; ++i) {
        auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(i));
        if (pt) { c[i*2]=pt->getX(); c[i*2+1]=pt->getY(); }
    }
    return MultiPoint<double>(c.data(), n, 2);
}
template <typename T> template <typename U>
MultiPoint<double> MultiPoint<T>::symmetric_difference(const Point<U>& o) const {
    auto res = detail::geos_sym_difference(geos_multipoint_.get(), o.geos_point_.get());
    if (!res || res->isEmpty()) return MultiPoint<double>();
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(res.get());
    if (!mpt) return MultiPoint<double>();
    size_t n = mpt->getNumGeometries();
    std::vector<double> c(n * 2);
    for (size_t i = 0; i < n; ++i) {
        auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(i));
        if (pt) { c[i*2]=pt->getX(); c[i*2+1]=pt->getY(); }
    }
    return MultiPoint<double>(c.data(), n, 2);
}
template <typename T>
MultiPoint<double> MultiPoint<T>::simplify(double tol) const {
    auto res = detail::geos_simplify(geos_multipoint_.get(), tol);
    if (!res || res->isEmpty()) return MultiPoint<double>();
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(res.get());
    if (!mpt) return MultiPoint<double>();
    size_t n = mpt->getNumGeometries();
    std::vector<double> c(n * 2);
    for (size_t i = 0; i < n; ++i) {
        auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(i));
        if (pt) { c[i*2]=pt->getX(); c[i*2+1]=pt->getY(); }
    }
    return MultiPoint<double>(c.data(), n, 2);
}

// -- Accessors ---------------------------------------------------------------
template <typename T> std::string MultiPoint<T>::wkt() const { return detail::geos_to_wkt(geos_multipoint_.get()); }
template <typename T> std::string MultiPoint<T>::wkb_hex() const { return detail::geos_to_wkb_hex(geos_multipoint_.get()); }
template <typename T> std::string MultiPoint<T>::type() const { return "MultiPoint"; }
template <typename T> std::string MultiPoint<T>::geom_type() const { return detail::geos_geom_type(geos_multipoint_.get()); }
template <typename T> bool MultiPoint<T>::has_z() const { return detail::geos_has_z(geos_multipoint_.get()); }

// -- Properties ---------------------------------------------------------------
template <typename T> bool MultiPoint<T>::is_empty() const { return detail::geos_is_empty(geos_multipoint_.get()); }
template <typename T> bool MultiPoint<T>::is_simple() const { return detail::geos_is_simple(geos_multipoint_.get()); }
template <typename T> bool MultiPoint<T>::is_valid() const { return detail::geos_is_valid(geos_multipoint_.get()); }
template <typename T> double MultiPoint<T>::area() const { return 0.0; }
template <typename T> double MultiPoint<T>::length() const { return 0.0; }
template <typename T> std::vector<double> MultiPoint<T>::bounds() const { return detail::geos_bounds(geos_multipoint_.get()); }

// -- Topology ------------------------------------------------------------------
template <typename T>
Point<double> MultiPoint<T>::centroid() const {
    auto c = geos_multipoint_->getCentroid();
    if (!c) return Point<double>(0, 0);
    auto* coord = c->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

template <typename T>
MultiPoint<double> MultiPoint<T>::convex_hull() const {
    auto res = detail::geos_convex_hull(geos_multipoint_.get());
    if (!res || res->isEmpty()) return MultiPoint<double>();
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(res.get());
    if (mpt) {
        size_t n = mpt->getNumGeometries();
        std::vector<double> c(n*2);
        for (size_t i = 0; i < n; ++i) {
            auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(i));
            if (pt) { c[i*2]=pt->getX(); c[i*2+1]=pt->getY(); }
        }
        return MultiPoint<double>(c.data(), n, 2);
    }
    return MultiPoint<double>();
}

template <typename T>
MultiPoint<double> MultiPoint<T>::envelope() const {
    auto res = detail::geos_envelope(geos_multipoint_.get());
    // Envelope returns a Polygon, not MultiPoint; return empty
    return MultiPoint<double>();
}

template <typename T>
MultiPoint<double> MultiPoint<T>::representative_point() const {
    auto res = detail::geos_representative_point(geos_multipoint_.get());
    if (!res) return MultiPoint<double>();
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (pt) {
        double c[2] = {pt->getX(), pt->getY()};
        return MultiPoint<double>(c, 1, 2);
    }
    return MultiPoint<double>();
}

template <typename T>
MultiPoint<double> MultiPoint<T>::buffer(double distance) const {
    auto buf = geos_multipoint_->buffer(distance, 16);
    if (!buf || buf->isEmpty()) return MultiPoint<double>();
    auto* mpt = dynamic_cast<geos::geom::MultiPoint*>(buf.get());
    if (!mpt) {
        if (buf->getGeometryTypeId() == geos::geom::GEOS_POINT) {
            auto* pt = dynamic_cast<const geos::geom::Point*>(buf.get());
            if (pt) { double c[2]={pt->getX(), pt->getY()}; return MultiPoint<double>(c,1,2); }
        }
        return MultiPoint<double>();
    }
    size_t n = mpt->getNumGeometries();
    std::vector<double> c(n*2);
    for (size_t i = 0; i < n; ++i) {
        auto* pt = dynamic_cast<const geos::geom::Point*>(mpt->getGeometryN(i));
        if (pt) { c[i*2]=pt->getX(); c[i*2+1]=pt->getY(); }
    }
    return MultiPoint<double>(c.data(), n, 2);
}

template <typename T>
void MultiPoint<T>::normalize() { geos_multipoint_->normalize(); }

} // namespace geometry
} // namespace shapely
