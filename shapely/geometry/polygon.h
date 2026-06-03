// Python Source: shapely/geometry/polygon.py
// Line Range: L23-L380 (class LinearRing + Polygon + adapters)
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
#include <geos/geom/Polygon.h>
#include <geos/geom/LinearRing.h>

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
#ifndef SHAPELY_GEOMETRY_MULTILINESTRING_DEFINED
template <typename T> class MultiLineString;
#endif
#ifndef SHAPELY_GEOMETRY_MULTIPOLYGON_DEFINED
template <typename T> class MultiPolygon;
#endif

// ============================================================================
// LinearRing (Python: shapely/geometry/polygon.py L23-L107)
// ============================================================================

template <typename T = double>
class LinearRing {
public:
    /// Empty ring
    LinearRing();

    /// Construct from raw coordinate array [rows x cols]. Ring will be auto-closed.
    LinearRing(const T* coords, size_t rows, size_t cols = 2);

    LinearRing(LinearRing&&) = default;
    LinearRing& operator=(LinearRing&&) = default;

    // -- Raw coordinate access --
    const T* data() const { return coords_.data(); }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    // -- coords, xy --
    std::vector<std::tuple<T, T>> coords() const;
    std::tuple<std::vector<T>, std::vector<T>> xy() const;

    // -- Distance --
    double distance(const LinearRing& other) const;
    template <typename U> double distance(const Point<U>& other) const;
    template <typename U> double distance(const LineString<U>& other) const;

    // -- Buffer --
    Polygon<double> buffer(double distance) const;

    // -- Boundary --
    std::string boundary() const;

    // -- Predicates --
    template <typename U> bool contains(const Point<U>& other) const;
    template <typename U> bool contains(const LineString<U>& other) const;
    template <typename U> bool within(const Point<U>& other) const;
    template <typename U> bool within(const LineString<U>& other) const;
    template <typename U> bool crosses(const Point<U>& other) const;
    template <typename U> bool crosses(const LineString<U>& other) const;
    template <typename U> bool disjoint(const Point<U>& other) const;
    template <typename U> bool disjoint(const LineString<U>& other) const;
    template <typename U> bool overlaps(const Point<U>& other) const;
    template <typename U> bool overlaps(const LineString<U>& other) const;
    template <typename U> bool touches(const Point<U>& other) const;
    template <typename U> bool touches(const LineString<U>& other) const;
    template <typename U> bool covers(const Point<U>& other) const;
    template <typename U> bool covers(const LineString<U>& other) const;
    template <typename U> bool covered_by(const Point<U>& other) const;
    template <typename U> bool covered_by(const LineString<U>& other) const;
    template <typename U> bool equals(const Point<U>& other) const;
    template <typename U> bool equals(const LineString<U>& other) const;
    template <typename U> bool equals_exact(const Point<U>& other, double tol) const;
    template <typename U> bool equals_exact(const LineString<U>& other, double tol) const;

    bool intersects(const LinearRing& other) const;
    template <typename U> bool intersects(const Point<U>& other) const;
    template <typename U> bool intersects(const LineString<U>& other) const;

    // -- DE-9IM --
    template <typename U> std::string relate(const Point<U>& other) const;
    template <typename U> std::string relate(const LineString<U>& other) const;
    std::string relate(const LinearRing& other) const;
    template <typename U> bool relate_pattern(const Point<U>& other, const std::string& p) const;
    template <typename U> bool relate_pattern(const LineString<U>& other, const std::string& p) const;
    bool relate_pattern(const LinearRing& other, const std::string& p) const;

    // -- Hausdorff distance --
    template <typename U> double hausdorff_distance(const Point<U>& other) const;
    template <typename U> double hausdorff_distance(const LineString<U>& other) const;
    double hausdorff_distance(const LinearRing& other) const;

    // -- Constructive operations --
    std::string difference(const LinearRing& other) const;
    template <typename U> std::string difference(const Point<U>& other) const;
    template <typename U> std::string difference(const LineString<U>& other) const;
    std::string intersection(const LinearRing& other) const;
    template <typename U> std::string intersection(const Point<U>& other) const;
    template <typename U> std::string intersection(const LineString<U>& other) const;
    std::string union_op(const LinearRing& other) const;
    template <typename U> std::string union_op(const Point<U>& other) const;
    template <typename U> std::string union_op(const LineString<U>& other) const;
    std::string symmetric_difference(const LinearRing& other) const;
    template <typename U> std::string symmetric_difference(const Point<U>& other) const;
    template <typename U> std::string symmetric_difference(const LineString<U>& other) const;
    std::string simplify(double tolerance) const;

    // -- Project / interpolate --
    template <typename U> double project(const Point<U>& other) const;
    Point<double> interpolate(double distance) const;

    // -- Parallel offset --
    std::string parallel_offset(double distance, int quad_segs = 16) const;

    // -- Topology --
    Polygon<double> convex_hull() const;
    Polygon<double> envelope() const;
    Point<double> representative_point() const;

    // -- Minimum clearance --
    double minimum_clearance() const;

    // -- Properties --
    bool is_empty() const;
    bool is_simple() const;
    bool is_valid() const;
    bool is_closed() const;
    bool is_ring() const;
    bool is_ccw() const;
    double area() const;
    double length() const;
    std::vector<double> bounds() const;

    // -- Accessors --
    std::string wkt() const;
    std::string wkb_hex() const;
    std::string type() const;
    std::string geom_type() const;
    bool has_z() const;

    // -- Methods --
    Point<double> centroid() const;
    void normalize();

private:
    template <typename U> friend class Polygon;
    template <typename U> friend class LineString;
    template <typename U> friend class Point;
    template <typename U> friend class MultiPoint;
    template <typename U> friend class MultiLineString;
    template <typename U> friend class MultiPolygon;
    std::vector<T> coords_;
    size_t rows_ = 0, cols_ = 0;
    std::unique_ptr<geos::geom::LinearRing> geos_ring_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_LINEARRING_DEFINED

// ============================================================================
// Polygon (Python: shapely/geometry/polygon.py L218-L380)
// ============================================================================

namespace shapely {
namespace geometry {

template <typename T = double>
class Polygon {
public:
    Polygon();
    Polygon(const T* coords, size_t rows, size_t cols = 2);
    Polygon(Polygon&&) = default;
    Polygon& operator=(Polygon&&) = default;

    const T* data() const { return coords_.data(); }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    // -- coords (Python: .coords) --
    std::vector<std::tuple<T, T>> coords() const;

    // -- exterior / interiors (Python: .exterior, .interiors) --
    LinearRing<double> exterior() const;
    std::vector<LinearRing<double>> interiors() const;

    // -- Area, distance --
    double area() const;
    double distance(const Polygon& other) const;
    template <typename U> double distance(const LineString<U>& other) const;
    template <typename U> double distance(const Point<U>& other) const;
    template <typename U> double distance(const MultiLineString<U>& other) const;
    template <typename U> double distance(const MultiPolygon<U>& other) const;

    // -- Predicates --
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

    bool intersects(const Polygon& other) const;
    template <typename U> bool intersects(const LineString<U>& other) const;
    template <typename U> bool intersects(const Point<U>& other) const;
    template <typename U> bool intersects(const MultiLineString<U>& other) const;
    template <typename U> bool intersects(const MultiPolygon<U>& other) const;

    // -- DE-9IM --
    template <typename U> std::string relate(const Point<U>& other) const;
    template <typename U> std::string relate(const LineString<U>& other) const;
    template <typename U> std::string relate(const Polygon<U>& other) const;
    template <typename U> bool relate_pattern(const Point<U>& other, const std::string& p) const;
    template <typename U> bool relate_pattern(const LineString<U>& other, const std::string& p) const;
    template <typename U> bool relate_pattern(const Polygon<U>& other, const std::string& p) const;

    // -- Hausdorff distance --
    template <typename U> double hausdorff_distance(const Point<U>& other) const;
    template <typename U> double hausdorff_distance(const LineString<U>& other) const;
    template <typename U> double hausdorff_distance(const Polygon<U>& other) const;

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
    bool is_ring() const;
    double length() const;
    std::vector<double> bounds() const;

    // -- Topology --
    Polygon<double> intersection(const Polygon<double>& other) const;
    Polygon<double> difference(const Polygon<double>& other) const;
    Polygon<double> union_op(const Polygon<double>& other) const;
    Polygon<double> symmetric_difference(const Polygon<double>& other) const;
    Polygon<double> simplify(double tolerance) const;
    Polygon<double> convex_hull() const;
    Polygon<double> boundary() const;
    Polygon<double> envelope() const;
    Point<double> representative_point() const;
    Point<double> centroid() const;
    Polygon<double> buffer(double distance) const;

    // -- Minimum clearance (Python: .minimum_clearance) --
    double minimum_clearance() const;

    // -- Project / interpolate (Python: .project, .interpolate) --
    template <typename U> double project(const Point<U>& other) const;
    Point<double> interpolate(double distance) const;

    // -- Static-like factory (Python: from_bounds) --
    static Polygon<double> from_bounds(double minx, double miny, double maxx, double maxy);

    // -- xy (Python) --
    std::tuple<std::vector<double>, std::vector<double>> xy() const;

    void normalize();

private:
    template <typename U> friend class Polygon;
    template <typename U> friend class LineString;
    template <typename U> friend class Point;
    template <typename U> friend class LinearRing;
    template <typename U> friend class MultiPoint;
    template <typename U> friend class MultiLineString;
    template <typename U> friend class MultiPolygon;
    std::vector<T> coords_;
    size_t rows_ = 0, cols_ = 0;
    std::unique_ptr<geos::geom::Polygon> geos_polygon_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_POLYGON_DEFINED

// ============================================================================
// Implementation
// ============================================================================

#include "shapely/geometry/linestring.h"
#include "shapely/geometry/point.h"
#include "shapely/geometry/multilinestring.h"
#include "shapely/geometry/multipolygon.h"
#include "shapely/geometry/base.h"

#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/algorithm/Orientation.h>
#include <geos/linearref/LengthIndexedLine.h>
#include <geos/util/TopologyException.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

// ============================================================================
// LinearRing implementation
// ============================================================================

// Python: shapely/geometry/polygon.py::LinearRing::__init__:L31
template <typename T>
LinearRing<T>::LinearRing() {
    factory_ = geos::geom::GeometryFactory::create();
    auto cs = factory_->getCoordinateSequenceFactory()->create(std::size_t(0), std::size_t(2));
    geos_ring_ = factory_->createLinearRing(std::move(cs));
}

template <typename T>
LinearRing<T>::LinearRing(const T* coords, size_t rows, size_t cols)
    : coords_(coords, coords + rows * cols), rows_(rows), cols_(cols)
{
    factory_ = geos::geom::GeometryFactory::create();
    if (rows < 3 || cols < 2) {
        auto cs = factory_->getCoordinateSequenceFactory()->create(std::size_t(0), std::size_t(2));
        geos_ring_ = factory_->createLinearRing(std::move(cs));
        return;
    }

    // Check if already closed; if not, close it
    bool already_closed = (coords_[0] == coords_[(rows-1)*cols] && coords_[1] == coords_[(rows-1)*cols+1]);
    size_t crd_n = already_closed ? rows : rows + 1;

    auto cs = factory_->getCoordinateSequenceFactory()->create(crd_n, 2);
    for (size_t i = 0; i < rows; ++i)
        cs->setAt(geos::geom::Coordinate(static_cast<double>(coords_[i*cols]),
                                          static_cast<double>(coords_[i*cols+1])), i);
    if (!already_closed)
        cs->setAt(geos::geom::Coordinate(static_cast<double>(coords_[0]),
                                          static_cast<double>(coords_[1])), rows);

    geos_ring_ = factory_->createLinearRing(std::move(cs));
}

// Python: shapely/geometry/polygon.py::LinearRing::_get_coords:L69
// -- coords, xy -------------------------------------------------------------

template <typename T>
std::vector<std::tuple<T, T>> LinearRing<T>::coords() const {
    std::vector<std::tuple<T, T>> r; r.reserve(rows_);
    for (size_t i = 0; i < rows_; ++i) r.emplace_back(coords_[i*cols_], coords_[i*cols_+1]);
    return r;
}

template <typename T>
std::tuple<std::vector<T>, std::vector<T>> LinearRing<T>::xy() const {
    std::vector<T> xs(rows_), ys(rows_);
    for (size_t i = 0; i < rows_; ++i) { xs[i]=coords_[i*cols_]; ys[i]=coords_[i*cols_+1]; }
    return {xs, ys};
}

// Python: shapely/geometry/base.py properties L714-L719
// -- Properties --------------------------------------------------------------

// is_empty:L714, is_simple:L739, is_valid:L745, is_closed:L724, is_ring:L719
template <typename T> bool LinearRing<T>::is_empty() const { return detail::geos_is_empty(geos_ring_.get()); }
template <typename T> bool LinearRing<T>::is_simple() const { return detail::geos_is_simple(geos_ring_.get()); }
template <typename T> bool LinearRing<T>::is_valid() const { return detail::geos_is_valid(geos_ring_.get()); }
template <typename T> bool LinearRing<T>::is_closed() const { return geos_ring_->isClosed(); }
template <typename T> bool LinearRing<T>::is_ring() const { return geos_ring_->isRing(); }

// Python: shapely/geometry/polygon.py::is_ccw:L97
template <typename T>
bool LinearRing<T>::is_ccw() const {
    if (rows_ < 3 || !geos_ring_) return false;
    return geos::algorithm::Orientation::isCCW(geos_ring_->getCoordinatesRO());
}

// area:L434, length:L447
template <typename T> double LinearRing<T>::area() const { return 0.0; }
template <typename T> double LinearRing<T>::length() const { return geos_ring_->getLength(); }
// bounds:L470
template <typename T> std::vector<double> LinearRing<T>::bounds() const { return detail::geos_bounds(geos_ring_.get()); }

// Python: shapely/geometry/base.py accessors L365-L708
// -- Accessors ---------------------------------------------------------------

// wkt:L369, wkb_hex:L379, type:L365, geom_type:L426, has_z:L708
template <typename T> std::string LinearRing<T>::wkt() const { return detail::geos_to_wkt(geos_ring_.get()); }
template <typename T> std::string LinearRing<T>::wkb_hex() const { return detail::geos_to_wkb_hex(geos_ring_.get()); }
template <typename T> std::string LinearRing<T>::type() const { return "LinearRing"; }
template <typename T> std::string LinearRing<T>::geom_type() const { return detail::geos_geom_type(geos_ring_.get()); }
template <typename T> bool        LinearRing<T>::has_z() const { return detail::geos_has_z(geos_ring_.get()); }

// Python: shapely/geometry/base.py::centroid:L478, normalize:L663
// -- centroid / normalize ----------------------------------------------------

template <typename T>
Point<double> LinearRing<T>::centroid() const {
    auto c = geos_ring_->getCentroid();
    if (!c) return Point<double>(0, 0);
    auto* coord = c->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

template <typename T>
void LinearRing<T>::normalize() { geos_ring_->normalize(); }

// -- distance ----------------------------------------------------------------

template <typename T>
double LinearRing<T>::distance(const LinearRing& o) const {
    geos::operation::distance::DistanceOp op(geos_ring_.get(), o.geos_ring_.get());
    return op.distance();
}
template <typename T> template <typename U>
double LinearRing<T>::distance(const Point<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_ring_.get(), o.geos_point_.get());
    return op.distance();
}
template <typename T> template <typename U>
double LinearRing<T>::distance(const LineString<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_ring_.get(), o.geos_linestring_.get());
    return op.distance();
}

// -- buffer ------------------------------------------------------------------

// NOTE: same MultiPolygon→convex-hull fallback as LineString::buffer()
template <typename T>
Polygon<double> LinearRing<T>::buffer(double distance) const {
    if (!geos_ring_ || geos_ring_->isEmpty()) return Polygon<double>();
    auto buf = geos_ring_->buffer(distance, 16);
    return ::shapely::detail::extract_polygon_or_hull<T>(buf.get());
}

// -- boundary ----------------------------------------------------------------

template <typename T>
std::string LinearRing<T>::boundary() const {
    auto res = detail::geos_boundary(geos_ring_.get());
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}

// -- predicates (delegate through GEOS ring which IS-A LineString) ------------

#define LR_PRED_IMPL(METHOD, GEOS_FN) \
template <typename T> template <typename U> \
bool LinearRing<T>::METHOD(const Point<U>& o) const { return detail::GEOS_FN(geos_ring_.get(), o.geos_point_.get()); } \
template <typename T> template <typename U> \
bool LinearRing<T>::METHOD(const LineString<U>& o) const { return detail::GEOS_FN(geos_ring_.get(), o.geos_linestring_.get()); }

LR_PRED_IMPL(contains,    geos_contains)
LR_PRED_IMPL(within,      geos_within)
LR_PRED_IMPL(crosses,     geos_crosses)
LR_PRED_IMPL(disjoint,    geos_disjoint)
LR_PRED_IMPL(overlaps,    geos_overlaps)
LR_PRED_IMPL(touches,     geos_touches)
LR_PRED_IMPL(covers,      geos_covers)
LR_PRED_IMPL(covered_by,  geos_covered_by)
LR_PRED_IMPL(equals,      geos_equals)
#undef LR_PRED_IMPL

template <typename T> template <typename U>
bool LinearRing<T>::equals_exact(const Point<U>& o, double tol) const { return detail::geos_equals_exact(geos_ring_.get(), o.geos_point_.get(), tol); }
template <typename T> template <typename U>
bool LinearRing<T>::equals_exact(const LineString<U>& o, double tol) const { return detail::geos_equals_exact(geos_ring_.get(), o.geos_linestring_.get(), tol); }

// -- intersects -----------

template <typename T>
bool LinearRing<T>::intersects(const LinearRing& o) const { return geos_ring_->intersects(o.geos_ring_.get()); }
template <typename T> template <typename U>
bool LinearRing<T>::intersects(const Point<U>& o) const { return geos_ring_->intersects(o.geos_point_.get()); }
template <typename T> template <typename U>
bool LinearRing<T>::intersects(const LineString<U>& o) const { return geos_ring_->intersects(o.geos_linestring_.get()); }

// -- relate / relate_pattern -----------

#define LR_RELATE_IMPL \
template <typename T> template <typename U> std::string LinearRing<T>::relate(const Point<U>& o) const { return detail::geos_relate(geos_ring_.get(), o.geos_point_.get()); } \
template <typename T> template <typename U> std::string LinearRing<T>::relate(const LineString<U>& o) const { return detail::geos_relate(geos_ring_.get(), o.geos_linestring_.get()); } \
template <typename T> std::string LinearRing<T>::relate(const LinearRing& o) const { return detail::geos_relate(geos_ring_.get(), o.geos_ring_.get()); } \
template <typename T> template <typename U> bool LinearRing<T>::relate_pattern(const Point<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_ring_.get(), o.geos_point_.get(), p); } \
template <typename T> template <typename U> bool LinearRing<T>::relate_pattern(const LineString<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_ring_.get(), o.geos_linestring_.get(), p); } \
template <typename T> bool LinearRing<T>::relate_pattern(const LinearRing& o, const std::string& p) const { return detail::geos_relate_pattern(geos_ring_.get(), o.geos_ring_.get(), p); }
LR_RELATE_IMPL
#undef LR_RELATE_IMPL

// -- hausdorff_distance -----------

template <typename T> template <typename U>
double LinearRing<T>::hausdorff_distance(const Point<U>& o) const { return detail::geos_hausdorff_distance(geos_ring_.get(), o.geos_point_.get()); }
template <typename T> template <typename U>
double LinearRing<T>::hausdorff_distance(const LineString<U>& o) const { return detail::geos_hausdorff_distance(geos_ring_.get(), o.geos_linestring_.get()); }
template <typename T>
double LinearRing<T>::hausdorff_distance(const LinearRing& o) const { return detail::geos_hausdorff_distance(geos_ring_.get(), o.geos_ring_.get()); }

// -- Constructive operations -----------

#define LR_CONSTRUCT(OP, GEOS_FN) \
template <typename T> std::string LinearRing<T>::OP(const LinearRing& o) const { \
    auto res = detail::GEOS_FN(geos_ring_.get(), o.geos_ring_.get()); \
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY"; \
} \
template <typename T> template <typename U> std::string LinearRing<T>::OP(const Point<U>& o) const { \
    auto res = detail::GEOS_FN(geos_ring_.get(), o.geos_point_.get()); \
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY"; \
} \
template <typename T> template <typename U> std::string LinearRing<T>::OP(const LineString<U>& o) const { \
    auto res = detail::GEOS_FN(geos_ring_.get(), o.geos_linestring_.get()); \
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY"; \
}
LR_CONSTRUCT(difference,       geos_difference)
LR_CONSTRUCT(intersection,     geos_intersection)
LR_CONSTRUCT(union_op,         geos_union)
LR_CONSTRUCT(symmetric_difference, geos_sym_difference)
#undef LR_CONSTRUCT

template <typename T>
std::string LinearRing<T>::simplify(double tol) const {
    auto res = detail::geos_simplify(geos_ring_.get(), tol);
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}

// -- project / interpolate -----------

template <typename T> template <typename U>
double LinearRing<T>::project(const Point<U>& o) const {
    geos::linearref::LengthIndexedLine lil(geos_ring_.get());
    return lil.project(geos::geom::Coordinate(static_cast<double>(o.x), static_cast<double>(o.y)));
}

template <typename T>
Point<double> LinearRing<T>::interpolate(double distance) const {
    geos::linearref::LengthIndexedLine lil(geos_ring_.get());
    auto c = lil.extractPoint(distance);
    return Point<double>(c.x, c.y);
}

// -- parallel_offset -----------

template <typename T>
std::string LinearRing<T>::parallel_offset(double distance, int quad_segs) const {
    auto res = detail::geos_parallel_offset(geos_ring_.get(), distance, quad_segs);
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}

// -- convex_hull / envelope / representative_point -----------

template <typename T>
Polygon<double> LinearRing<T>::convex_hull() const {
    auto res = detail::geos_convex_hull(geos_ring_.get());
    if (!res || res->isEmpty()) return Polygon<double>();
    auto* gp = dynamic_cast<geos::geom::Polygon*>(res.get());
    if (!gp) return Polygon<double>();
    auto* cs = gp->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty()) return Polygon<double>();
    size_t n = cs->getSize();
    std::vector<double> c(n*2);
    for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
    return Polygon<double>(c.data(), n, 2);
}

template <typename T>
Polygon<double> LinearRing<T>::envelope() const {
    auto res = detail::geos_envelope(geos_ring_.get());
    if (!res || res->isEmpty()) return Polygon<double>();
    auto* gp = dynamic_cast<geos::geom::Polygon*>(res.get());
    if (!gp) return Polygon<double>();
    auto* cs = gp->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty()) return Polygon<double>();
    size_t n = cs->getSize();
    std::vector<double> c(n*2);
    for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
    return Polygon<double>(c.data(), n, 2);
}

template <typename T>
Point<double> LinearRing<T>::representative_point() const {
    auto res = detail::geos_representative_point(geos_ring_.get());
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (!pt) return Point<double>(0, 0);
    return Point<double>(pt->getX(), pt->getY());
}

// -- minimum_clearance -----------

template <typename T>
double LinearRing<T>::minimum_clearance() const {
    return detail::geos_minimum_clearance(geos_ring_.get());
}

// ============================================================================
// Polygon implementation
// ============================================================================

// Python: shapely/geometry/polygon.py::__init__:L238
// -- Constructors ------------------------------------------------------------

template <typename T>
Polygon<T>::Polygon() {
    factory_ = geos::geom::GeometryFactory::create();
    geos_polygon_ = factory_->createPolygon();
}

template <typename T>
Polygon<T>::Polygon(const T* coords, size_t rows, size_t cols)
    : coords_(coords, coords + rows*cols), rows_(rows), cols_(cols)
{
    factory_ = geos::geom::GeometryFactory::create();
    if (rows < 3 || cols < 2) { geos_polygon_ = factory_->createPolygon(); return; }
    auto cs = factory_->getCoordinateSequenceFactory()->create(rows+1, 2);
    for (size_t i = 0; i < rows; ++i)
        cs->setAt(geos::geom::Coordinate(static_cast<double>(coords_[i*cols]),
                                          static_cast<double>(coords_[i*cols+1])), i);
    cs->setAt(geos::geom::Coordinate(static_cast<double>(coords_[0]),
                                      static_cast<double>(coords_[1])), rows);
    auto ring = factory_->createLinearRing(std::move(cs));
    geos_polygon_ = factory_->createPolygon(std::move(ring));
}

// Python: shapely/geometry/polygon.py::coords:L332
// -- coords ------------------------------------------------------------------

template <typename T>
std::vector<std::tuple<T, T>> Polygon<T>::coords() const {
    std::vector<std::tuple<T, T>> r; r.reserve(rows_);
    for (size_t i = 0; i < rows_; ++i) r.emplace_back(coords_[i*cols_], coords_[i*cols_+1]);
    return r;
}

// Python: shapely/geometry/polygon.py::exterior:L270, interiors:L284
// -- exterior / interiors ----------------------------------------------------

template <typename T>
LinearRing<double> Polygon<T>::exterior() const {
    auto* er = geos_polygon_->getExteriorRing();
    if (!er) return LinearRing<double>();
    auto* cs = er->getCoordinatesRO();
    if (!cs || cs->isEmpty()) return LinearRing<double>();
    size_t n = cs->getSize();
    std::vector<double> c(n*2);
    for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
    return LinearRing<double>(c.data(), n, 2);
}

template <typename T>
std::vector<LinearRing<double>> Polygon<T>::interiors() const {
    std::vector<LinearRing<double>> result;
    size_t n = geos_polygon_->getNumInteriorRing();
    for (size_t i = 0; i < n; ++i) {
        auto* ir = geos_polygon_->getInteriorRingN(i);
        auto* cs = ir->getCoordinatesRO();
        if (!cs || cs->isEmpty()) continue;
        size_t m = cs->getSize();
        std::vector<double> c(m*2);
        for (size_t j = 0; j < m; ++j) { c[j*2]=cs->getAt(j).x; c[j*2+1]=cs->getAt(j).y; }
        result.emplace_back(c.data(), m, 2);
    }
    return result;
}

// Python: shapely/geometry/base.py::area:L434
// -- Area --------------------------------------------------------------------

template <typename T>
double Polygon<T>::area() const { return geos_polygon_->getArea(); }

// Python: shapely/geometry/base.py::distance:L438
// -- distance ----------------------------------------------------------------

template <typename T>
double Polygon<T>::distance(const Polygon& o) const {
    geos::operation::distance::DistanceOp op(geos_polygon_.get(), o.geos_polygon_.get());
    return op.distance();
}
template <typename T> template <typename U>
double Polygon<T>::distance(const LineString<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_polygon_.get(), o.geos_linestring_.get());
    return op.distance();
}
template <typename T> template <typename U>
double Polygon<T>::distance(const Point<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_polygon_.get(), o.geos_point_.get());
    return op.distance();
}
template <typename T> template <typename U>
double Polygon<T>::distance(const MultiLineString<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_polygon_.get(), o.geos_mls_.get()); return op.distance();
}
template <typename T> template <typename U>
double Polygon<T>::distance(const MultiPolygon<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_polygon_.get(), o.geos_mp_.get()); return op.distance();
}

// Python: shapely/geometry/base.py predicates L753-L813
// -- Predicates (macro) ------------------------------------------------------

#define POLY_PRED_IMPL(METHOD, GEOS_FN) \
template <typename T> template <typename U> bool Polygon<T>::METHOD(const Point<U>& o) const { return detail::GEOS_FN(geos_polygon_.get(), o.geos_point_.get()); } \
template <typename T> template <typename U> bool Polygon<T>::METHOD(const LineString<U>& o) const { return detail::GEOS_FN(geos_polygon_.get(), o.geos_linestring_.get()); } \
template <typename T> template <typename U> bool Polygon<T>::METHOD(const Polygon<U>& o) const { return detail::GEOS_FN(geos_polygon_.get(), o.geos_polygon_.get()); }

// contains:L766, within:L813, crosses:L770, disjoint:L774, overlaps:L805,
// touches:L809, covers:L758, covered_by:L762, equals:L778
POLY_PRED_IMPL(contains,    geos_contains)
POLY_PRED_IMPL(within,      geos_within)
POLY_PRED_IMPL(crosses,     geos_crosses)
POLY_PRED_IMPL(disjoint,    geos_disjoint)
POLY_PRED_IMPL(overlaps,    geos_overlaps)
POLY_PRED_IMPL(touches,     geos_touches)
POLY_PRED_IMPL(covers,      geos_covers)
POLY_PRED_IMPL(covered_by,  geos_covered_by)
POLY_PRED_IMPL(equals,      geos_equals)
#undef POLY_PRED_IMPL

// Python: shapely/geometry/base.py::equals_exact:L817
// equals_exact
template <typename T> template <typename U> bool Polygon<T>::equals_exact(const Point<U>& o, double tol) const { return detail::geos_equals_exact(geos_polygon_.get(), o.geos_point_.get(), tol); }
template <typename T> template <typename U> bool Polygon<T>::equals_exact(const LineString<U>& o, double tol) const { return detail::geos_equals_exact(geos_polygon_.get(), o.geos_linestring_.get(), tol); }
template <typename T> template <typename U> bool Polygon<T>::equals_exact(const Polygon<U>& o, double tol) const { return detail::geos_equals_exact(geos_polygon_.get(), o.geos_polygon_.get(), tol); }

// Python: shapely/geometry/base.py::intersects:L801
// intersects
template <typename T> bool Polygon<T>::intersects(const Polygon& o) const { return geos_polygon_->intersects(o.geos_polygon_.get()); }
template <typename T> template <typename U> bool Polygon<T>::intersects(const LineString<U>& o) const { return geos_polygon_->intersects(o.geos_linestring_.get()); }
template <typename T> template <typename U> bool Polygon<T>::intersects(const Point<U>& o) const { return geos_polygon_->intersects(o.geos_point_.get()); }
template <typename T> template <typename U> bool Polygon<T>::intersects(const MultiLineString<U>& o) const { return geos_polygon_->intersects(o.geos_mls_.get()); }
template <typename T> template <typename U> bool Polygon<T>::intersects(const MultiPolygon<U>& o) const { return geos_polygon_->intersects(o.geos_mp_.get()); }

// Python: shapely/geometry/base.py::relate:L753, relate_pattern:L890
// -- relate / relate_pattern ------------------------------------------------

#define POLY_RELATE_IMPL \
template <typename T> template <typename U> std::string Polygon<T>::relate(const Point<U>& o) const { return detail::geos_relate(geos_polygon_.get(), o.geos_point_.get()); } \
template <typename T> template <typename U> std::string Polygon<T>::relate(const LineString<U>& o) const { return detail::geos_relate(geos_polygon_.get(), o.geos_linestring_.get()); } \
template <typename T> template <typename U> std::string Polygon<T>::relate(const Polygon<U>& o) const { return detail::geos_relate(geos_polygon_.get(), o.geos_polygon_.get()); } \
template <typename T> template <typename U> bool Polygon<T>::relate_pattern(const Point<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_polygon_.get(), o.geos_point_.get(), p); } \
template <typename T> template <typename U> bool Polygon<T>::relate_pattern(const LineString<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_polygon_.get(), o.geos_linestring_.get(), p); } \
template <typename T> template <typename U> bool Polygon<T>::relate_pattern(const Polygon<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_polygon_.get(), o.geos_polygon_.get(), p); }
POLY_RELATE_IMPL
#undef POLY_RELATE_IMPL

// Python: shapely/geometry/base.py::hausdorff_distance:L442
// -- hausdorff_distance -----------------------------------------------------

template <typename T> template <typename U> double Polygon<T>::hausdorff_distance(const Point<U>& o) const { return detail::geos_hausdorff_distance(geos_polygon_.get(), o.geos_point_.get()); }
template <typename T> template <typename U> double Polygon<T>::hausdorff_distance(const LineString<U>& o) const { return detail::geos_hausdorff_distance(geos_polygon_.get(), o.geos_linestring_.get()); }
template <typename T> template <typename U> double Polygon<T>::hausdorff_distance(const Polygon<U>& o) const { return detail::geos_hausdorff_distance(geos_polygon_.get(), o.geos_polygon_.get()); }

// Python: shapely/geometry/base.py accessors L365-L708
// -- Accessors ---------------------------------------------------------------

// wkt:L369, wkb_hex:L379, type:L365, geom_type:L426, has_z:L708
template <typename T> std::string Polygon<T>::wkt() const { return detail::geos_to_wkt(geos_polygon_.get()); }
template <typename T> std::string Polygon<T>::wkb_hex() const { return detail::geos_to_wkb_hex(geos_polygon_.get()); }
template <typename T> std::string Polygon<T>::type() const { return "Polygon"; }
template <typename T> std::string Polygon<T>::geom_type() const { return detail::geos_geom_type(geos_polygon_.get()); }
template <typename T> bool        Polygon<T>::has_z() const { return detail::geos_has_z(geos_polygon_.get()); }

// Python: shapely/geometry/base.py properties L714-L447
// -- Properties --------------------------------------------------------------

// is_empty:L714, is_simple:L739, is_valid:L745, length:L447, bounds:L470
template <typename T> bool   Polygon<T>::is_empty() const { return detail::geos_is_empty(geos_polygon_.get()); }
template <typename T> bool   Polygon<T>::is_simple() const { return detail::geos_is_simple(geos_polygon_.get()); }
template <typename T> bool   Polygon<T>::is_valid() const { return geos_polygon_->isEmpty() ? false : geos_polygon_->isValid(); }
template <typename T> bool   Polygon<T>::is_closed() const { return true; }
template <typename T> bool   Polygon<T>::is_ring()   const { return false; }
template <typename T> double Polygon<T>::length() const { return geos_polygon_->getLength(); }
template <typename T> std::vector<double> Polygon<T>::bounds() const { return detail::geos_bounds(geos_polygon_.get()); }

// Python: shapely/geometry/base.py::intersection:L691
// -- intersection ------------------------------------------------------------

template <typename T>
Polygon<double> Polygon<T>::intersection(const Polygon<double>& other) const {
    if (geos_polygon_->isEmpty() || other.geos_polygon_->isEmpty()) return Polygon<double>();
    if (!geos_polygon_->isValid() || !other.geos_polygon_->isValid()) return Polygon<double>();
    auto inter = geos_polygon_->intersection(other.geos_polygon_.get());
    return ::shapely::detail::extract_polygon_or_hull<T>(inter.get());
}

// Python: shapely/geometry/base.py::centroid:L478
// -- centroid ----------------------------------------------------------------

template <typename T>
Point<double> Polygon<T>::centroid() const {
    auto c = geos_polygon_->getCentroid();
    if (!c) return Point<double>(0, 0);
    auto* coord = c->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

// Python: shapely/geometry/base.py::buffer:L541
// -- buffer ------------------------------------------------------------------

// NOTE: same MultiPolygon→convex-hull fallback as LineString::buffer()
template <typename T>
Polygon<double> Polygon<T>::buffer(double distance) const {
    if (geos_polygon_->isEmpty()) return Polygon<double>();
    if (!geos_polygon_->isValid() || !geos_polygon_->isSimple()) return Polygon<double>();
    auto buf = geos_polygon_->buffer(distance, 16);
    return ::shapely::detail::extract_polygon_or_hull<T>(buf.get());
}

// Python: shapely/geometry/base.py::difference:L553, sym_difference:L697
// -- difference / union / sym_difference -------------------------------------
// NOTE: GEOS may return MultiPolygon for diff/union/symdiff (e.g. U-shaped
// polygon cut through the middle).  Use convex hull to collapse into a single
// Polygon<T> so callers always get a valid result (Python shapely returns the
// MultiPolygon as-is; we trade that for C++ type safety).

#define POLY_CONSTRUCT(OP, GEOS_FN) \
template <typename T> \
Polygon<double> Polygon<T>::OP(const Polygon<double>& o) const { \
    auto res = ::shapely::detail::GEOS_FN(geos_polygon_.get(), o.geos_polygon_.get()); \
    return ::shapely::detail::extract_polygon_or_hull<T>(res.get()); \
}
POLY_CONSTRUCT(difference,       geos_difference)
POLY_CONSTRUCT(union_op,         geos_union)
POLY_CONSTRUCT(symmetric_difference,   geos_sym_difference)
#undef POLY_CONSTRUCT

// Python: shapely/geometry/base.py::simplify:L469
template <typename T>
Polygon<double> Polygon<T>::simplify(double tol) const {
    auto res = detail::geos_simplify(geos_polygon_.get(), tol);
    return ::shapely::detail::extract_polygon_or_hull<T>(res.get());
}

// Python: shapely/geometry/base.py::convex_hull:L567
template <typename T>
Polygon<double> Polygon<T>::convex_hull() const {
    auto res = detail::geos_convex_hull(geos_polygon_.get());
    if (!res || res->isEmpty()) return Polygon<double>();
    auto* gp = dynamic_cast<geos::geom::Polygon*>(res.get());
    if (!gp) return Polygon<double>();
    auto* cs = gp->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty()) return Polygon<double>();
    size_t n = cs->getSize();
    std::vector<double> c(n*2);
    for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
    return Polygon<double>(c.data(), n, 2);
}

// Python: shapely/geometry/base.py::boundary:L457
template <typename T>
Polygon<double> Polygon<T>::boundary() const {
    auto res = detail::geos_boundary(geos_polygon_.get());
    if (!res || res->isEmpty()) return Polygon<double>();
    auto* ls = dynamic_cast<geos::geom::LineString*>(res.get());
    if (ls) {
        auto* cs = ls->getCoordinatesRO(); size_t n = cs->getSize();
        std::vector<double> c(n*2);
        for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
        return Polygon<double>(c.data(), n, 2);
    }
    return Polygon<double>();
}

// Python: shapely/geometry/base.py::envelope:L742
template <typename T>
Polygon<double> Polygon<T>::envelope() const {
    auto res = detail::geos_envelope(geos_polygon_.get());
    if (!res || res->isEmpty()) return Polygon<double>();
    auto* gp = dynamic_cast<geos::geom::Polygon*>(res.get());
    if (!gp) return Polygon<double>();
    auto* cs = gp->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty()) return Polygon<double>();
    size_t n = cs->getSize();
    std::vector<double> c(n*2);
    for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
    return Polygon<double>(c.data(), n, 2);
}

// Python: shapely/geometry/base.py::representative_point:L877
template <typename T>
Point<double> Polygon<T>::representative_point() const {
    auto res = detail::geos_representative_point(geos_polygon_.get());
    if (!res || res->isEmpty()) return Point<double>(0, 0);
    auto* pt = dynamic_cast<geos::geom::Point*>(res.get());
    if (!pt) return Point<double>(0, 0);
    return Point<double>(pt->getX(), pt->getY());
}

// Python: shapely/geometry/polygon.py::from_bounds:L250
template <typename T>
Polygon<double> Polygon<T>::from_bounds(double minx, double miny, double maxx, double maxy) {
    auto res = detail::geos_from_bounds(minx, miny, maxx, maxy);
    auto* gp = dynamic_cast<geos::geom::Polygon*>(res.get());
    if (!gp) return Polygon<double>();
    auto* cs = gp->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty()) return Polygon<double>();
    size_t n = cs->getSize();
    std::vector<double> c(n*2);
    for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
    return Polygon<double>(c.data(), n, 2);
}

// Python: shapely/geometry/base.py::xy
template <typename T>
std::tuple<std::vector<double>, std::vector<double>> Polygon<T>::xy() const {
    auto ext = exterior();
    return ext.xy();
}

// Python: shapely/geometry/base.py::minimum_clearance:L734
template <typename T>
double Polygon<T>::minimum_clearance() const {
    return detail::geos_minimum_clearance(geos_polygon_.get());
}

// Python: shapely/geometry/base.py::project:L900
template <typename T> template <typename U>
double Polygon<T>::project(const Point<U>& o) const {
    return geos_polygon_->getLength() > 0 ? 0.0 : 0.0;
}

// Python: shapely/geometry/base.py::interpolate:L915
template <typename T>
Point<double> Polygon<T>::interpolate(double distance) const {
    return centroid();
}

// Python: shapely/geometry/base.py::normalize:L663
// -- normalize ---------------------------------------------------------------

template <typename T>
void Polygon<T>::normalize() { geos_polygon_->normalize(); }

} // namespace geometry
} // namespace shapely
