// Python Source: shapely/geometry/linestring.py
// Line Range: L19-L180 (class LineString + LineStringAdapter)
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
#include <geos/geom/LineString.h>

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_POLYGON_DEFINED
template <typename T> class Polygon;
#endif
#ifndef SHAPELY_GEOMETRY_POINT_DEFINED
template <typename T> class Point;
#endif
#ifndef SHAPELY_GEOMETRY_LINEARRING_DEFINED
template <typename T> class LinearRing;
#endif

template <typename T = double>
class LineString {
public:
    LineString(const T* coords, size_t rows, size_t cols = 2);
    LineString(LineString&&) = default;
    LineString& operator=(LineString&&) = default;

    // -- Raw coordinate access --
    const T* data() const { return coords_.data(); }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    // -- coords, xy (Python: .coords, .xy) --
    std::vector<std::tuple<T, T>> coords() const;
    std::tuple<std::vector<T>, std::vector<T>> xy() const;

    // -- Distance --
    double distance(const LineString& other) const;
    template <typename U> double distance(const Polygon<U>& other) const;
    template <typename U> double distance(const Point<U>& other) const;

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

    bool intersects(const LineString& other) const;
    template <typename U> bool intersects(const Point<U>& other) const;
    template <typename U> bool intersects(const Polygon<U>& other) const;

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
    double area() const;
    double length() const;
    std::vector<double> bounds() const;

    // -- Topology --
    template <typename U> double project(const Point<U>& other) const;
    Point<double> interpolate(double distance) const;
    Point<double> centroid() const;
    Polygon<double> buffer(double distance) const;
    void normalize();

    // -- Constructive operations (Python: difference, union, intersection, sym_difference) --
    std::string difference(const LineString& other) const;
    template <typename U> std::string difference(const Point<U>& other) const;
    template <typename U> std::string difference(const Polygon<U>& other) const;
    std::string intersection(const LineString& other) const;
    template <typename U> std::string intersection(const Point<U>& other) const;
    template <typename U> std::string intersection(const Polygon<U>& other) const;
    std::string union_op(const LineString& other) const;
    template <typename U> std::string union_op(const Point<U>& other) const;
    template <typename U> std::string union_op(const Polygon<U>& other) const;
    std::string sym_difference(const LineString& other) const;
    template <typename U> std::string sym_difference(const Point<U>& other) const;
    template <typename U> std::string sym_difference(const Polygon<U>& other) const;
    std::string simplify(double tolerance) const;
    std::string parallel_offset(double distance, int quad_segs = 16) const;
    Polygon<double> convex_hull() const;

private:
    template <typename U> friend class Polygon;
    template <typename U> friend class Point;
    template <typename U> friend class LinearRing;
    std::vector<T> coords_;
    size_t rows_ = 0, cols_ = 0;
    std::unique_ptr<geos::geom::LineString> geos_linestring_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_LINESTRING_DEFINED

// ============================================================================
// Implementation
// ============================================================================

#include "shapely/geometry/point.h"
#include "shapely/geometry/polygon.h"
#include "shapely/geometry/base.h"

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

// Python: shapely/geometry/linestring.py::__init__:L27
// -- Constructor -------------------------------------------------------------

template <typename T>
LineString<T>::LineString(const T* coords, size_t rows, size_t cols)
    : coords_(coords, coords + rows * cols), rows_(rows), cols_(cols)
{
    if (cols < 2) throw std::runtime_error("LineString: cols must be >= 2");
    if (rows < 2) throw std::runtime_error("LineString: rows must be >= 2");
    factory_ = geos::geom::GeometryFactory::create();
    auto cs = factory_->getCoordinateSequenceFactory()->create(rows, 2);
    for (size_t i = 0; i < rows; ++i)
        cs->setAt(geos::geom::Coordinate(static_cast<double>(coords_[i*cols+0]),
                                          static_cast<double>(coords_[i*cols+1])), i);
    geos_linestring_ = factory_->createLineString(std::move(cs));
}

// Python: shapely/geometry/linestring.py::_get_coords:L115, xy:L134
// -- coords, xy -------------------------------------------------------------

template <typename T>
std::vector<std::tuple<T, T>> LineString<T>::coords() const {
    std::vector<std::tuple<T, T>> r; r.reserve(rows_);
    for (size_t i = 0; i < rows_; ++i) r.emplace_back(coords_[i*cols_], coords_[i*cols_+1]);
    return r;
}

template <typename T>
std::tuple<std::vector<T>, std::vector<T>> LineString<T>::xy() const {
    std::vector<T> xs(rows_), ys(rows_);
    for (size_t i = 0; i < rows_; ++i) { xs[i]=coords_[i*cols_]; ys[i]=coords_[i*cols_+1]; }
    return {xs, ys};
}

// Python: shapely/geometry/base.py::distance:L438
// -- distance ----------------------------------------------------------------

template <typename T>
double LineString<T>::distance(const LineString& o) const {
    geos::operation::distance::DistanceOp op(geos_linestring_.get(), o.geos_linestring_.get());
    return op.distance();
}
template <typename T> template <typename U>
double LineString<T>::distance(const Polygon<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_linestring_.get(), o.geos_polygon_.get());
    return op.distance();
}
template <typename T> template <typename U>
double LineString<T>::distance(const Point<U>& o) const {
    geos::operation::distance::DistanceOp op(geos_linestring_.get(), o.geos_point_.get());
    return op.distance();
}

// Python: shapely/geometry/base.py predicates L753-L813
// -- Predicates (macro) ------------------------------------------------------

#define LS_PRED_IMPL(METHOD, GEOS_FN) \
template <typename T> template <typename U> \
bool LineString<T>::METHOD(const Point<U>& o) const { return detail::GEOS_FN(geos_linestring_.get(), o.geos_point_.get()); } \
template <typename T> template <typename U> \
bool LineString<T>::METHOD(const LineString<U>& o) const { return detail::GEOS_FN(geos_linestring_.get(), o.geos_linestring_.get()); } \
template <typename T> template <typename U> \
bool LineString<T>::METHOD(const Polygon<U>& o) const { return detail::GEOS_FN(geos_linestring_.get(), o.geos_polygon_.get()); }

// contains:L766, within:L813, crosses:L770, disjoint:L774, overlaps:L805,
// touches:L809, covers:L758, covered_by:L762, equals:L778
LS_PRED_IMPL(contains,    geos_contains)
LS_PRED_IMPL(within,      geos_within)
LS_PRED_IMPL(crosses,     geos_crosses)
LS_PRED_IMPL(disjoint,    geos_disjoint)
LS_PRED_IMPL(overlaps,    geos_overlaps)
LS_PRED_IMPL(touches,     geos_touches)
LS_PRED_IMPL(covers,      geos_covers)
LS_PRED_IMPL(covered_by,  geos_covered_by)
LS_PRED_IMPL(equals,      geos_equals)
#undef LS_PRED_IMPL

// Python: shapely/geometry/base.py::equals_exact:L817
// equals_exact
template <typename T> template <typename U> bool LineString<T>::equals_exact(const Point<U>& o, double tol) const { return detail::geos_equals_exact(geos_linestring_.get(), o.geos_point_.get(), tol); }
template <typename T> template <typename U> bool LineString<T>::equals_exact(const LineString<U>& o, double tol) const { return detail::geos_equals_exact(geos_linestring_.get(), o.geos_linestring_.get(), tol); }
template <typename T> template <typename U> bool LineString<T>::equals_exact(const Polygon<U>& o, double tol) const { return detail::geos_equals_exact(geos_linestring_.get(), o.geos_polygon_.get(), tol); }

// Python: shapely/geometry/base.py::intersects:L801
// intersects
template <typename T> bool LineString<T>::intersects(const LineString& o) const { return geos_linestring_->intersects(o.geos_linestring_.get()); }
template <typename T> template <typename U> bool LineString<T>::intersects(const Point<U>& o) const { return geos_linestring_->intersects(o.geos_point_.get()); }
template <typename T> template <typename U> bool LineString<T>::intersects(const Polygon<U>& o) const { return geos_linestring_->intersects(o.geos_polygon_.get()); }

// Python: shapely/geometry/base.py::relate:L753, relate_pattern:L890
// -- relate / relate_pattern ------------------------------------------------

#define LS_RELATE_IMPL \
template <typename T> template <typename U> std::string LineString<T>::relate(const Point<U>& o) const { return detail::geos_relate(geos_linestring_.get(), o.geos_point_.get()); } \
template <typename T> template <typename U> std::string LineString<T>::relate(const LineString<U>& o) const { return detail::geos_relate(geos_linestring_.get(), o.geos_linestring_.get()); } \
template <typename T> template <typename U> std::string LineString<T>::relate(const Polygon<U>& o) const { return detail::geos_relate(geos_linestring_.get(), o.geos_polygon_.get()); } \
template <typename T> template <typename U> bool LineString<T>::relate_pattern(const Point<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_linestring_.get(), o.geos_point_.get(), p); } \
template <typename T> template <typename U> bool LineString<T>::relate_pattern(const LineString<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_linestring_.get(), o.geos_linestring_.get(), p); } \
template <typename T> template <typename U> bool LineString<T>::relate_pattern(const Polygon<U>& o, const std::string& p) const { return detail::geos_relate_pattern(geos_linestring_.get(), o.geos_polygon_.get(), p); }
LS_RELATE_IMPL
#undef LS_RELATE_IMPL

// Python: shapely/geometry/base.py::hausdorff_distance:L442
// -- hausdorff_distance -----------------------------------------------------

template <typename T> template <typename U> double LineString<T>::hausdorff_distance(const Point<U>& o) const { return detail::geos_hausdorff_distance(geos_linestring_.get(), o.geos_point_.get()); }
template <typename T> template <typename U> double LineString<T>::hausdorff_distance(const LineString<U>& o) const { return detail::geos_hausdorff_distance(geos_linestring_.get(), o.geos_linestring_.get()); }
template <typename T> template <typename U> double LineString<T>::hausdorff_distance(const Polygon<U>& o) const { return detail::geos_hausdorff_distance(geos_linestring_.get(), o.geos_polygon_.get()); }

// Python: shapely/geometry/base.py accessors L365-L447
// -- Accessors ---------------------------------------------------------------

// wkt:L369, wkb_hex:L379, type:L365, geom_type:L426, has_z:L708
template <typename T> std::string LineString<T>::wkt() const { return detail::geos_to_wkt(geos_linestring_.get()); }
template <typename T> std::string LineString<T>::wkb_hex() const { return detail::geos_to_wkb_hex(geos_linestring_.get()); }
template <typename T> std::string LineString<T>::type() const { return "LineString"; }
template <typename T> std::string LineString<T>::geom_type() const { return detail::geos_geom_type(geos_linestring_.get()); }
template <typename T> bool        LineString<T>::has_z() const { return detail::geos_has_z(geos_linestring_.get()); }

// Python: shapely/geometry/base.py properties L714-L447
// -- Properties --------------------------------------------------------------

// is_empty:L714, is_simple:L739, is_valid:L745, is_closed:L724, is_ring:L719
template <typename T> bool        LineString<T>::is_empty() const { return detail::geos_is_empty(geos_linestring_.get()); }
template <typename T> bool        LineString<T>::is_simple() const { return detail::geos_is_simple(geos_linestring_.get()); }
template <typename T> bool        LineString<T>::is_valid() const { return detail::geos_is_valid(geos_linestring_.get()); }
template <typename T> bool        LineString<T>::is_closed() const { return geos_linestring_->isClosed(); }
template <typename T> bool        LineString<T>::is_ring() const { return geos_linestring_->isRing(); }
template <typename T> double      LineString<T>::area() const { return 0.0; }
template <typename T> double      LineString<T>::length() const { return geos_linestring_->getLength(); }
template <typename T> std::vector<double> LineString<T>::bounds() const { return detail::geos_bounds(geos_linestring_.get()); }

// Python: shapely/geometry/base.py L900-L915, L478, L541, L663
// -- project / interpolate / centroid / buffer / normalize ------------------

// Python: base.py::project:L900
template <typename T> template <typename U>
double LineString<T>::project(const Point<U>& o) const {
    geos::linearref::LengthIndexedLine lil(geos_linestring_.get());
    return lil.project(geos::geom::Coordinate(static_cast<double>(o.x), static_cast<double>(o.y)));
}

// Python: base.py::interpolate:L915
template <typename T>
Point<double> LineString<T>::interpolate(double distance) const {
    geos::linearref::LengthIndexedLine lil(geos_linestring_.get());
    auto c = lil.extractPoint(distance);
    return Point<double>(c.x, c.y);
}

// Python: base.py::centroid:L478
template <typename T>
Point<double> LineString<T>::centroid() const {
    auto c = geos_linestring_->getCentroid();
    if (!c) return Point<double>(0, 0);
    auto* coord = c->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

// Python: base.py::buffer:L541
template <typename T>
Polygon<double> LineString<T>::buffer(double distance) const {
    if (!geos_linestring_ || geos_linestring_->isEmpty()) return Polygon<double>();
    auto buf = geos_linestring_->buffer(distance, 16);
    if (!buf || buf->isEmpty()) return Polygon<double>();
    const auto* poly = buf.get();
    if (poly->getGeometryTypeId() != geos::geom::GEOS_POLYGON) {
        if (poly->getNumGeometries() > 0) poly = poly->getGeometryN(0);
    }
    if (poly->getGeometryTypeId() != geos::geom::GEOS_POLYGON || poly->isEmpty()) return Polygon<double>();
    auto* gp = dynamic_cast<const geos::geom::Polygon*>(poly);
    if (!gp) return Polygon<double>();
    auto* cs = gp->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty()) return Polygon<double>();
    size_t n = cs->getSize();
    std::vector<double> c(n*2);
    for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
    return Polygon<double>(c.data(), n, 2);
}

// Python: shapely/geometry/base.py L553-L703
// -- Constructive operations -------------------------------------------------

#define LS_CONSTRUCT(OP, GEOS_FN) \
template <typename T> std::string LineString<T>::OP(const LineString& o) const { \
    auto res = detail::GEOS_FN(geos_linestring_.get(), o.geos_linestring_.get()); \
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY"; \
} \
template <typename T> template <typename U> std::string LineString<T>::OP(const Point<U>& o) const { \
    auto res = detail::GEOS_FN(geos_linestring_.get(), o.geos_point_.get()); \
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY"; \
} \
template <typename T> template <typename U> std::string LineString<T>::OP(const Polygon<U>& o) const { \
    auto res = detail::GEOS_FN(geos_linestring_.get(), o.geos_polygon_.get()); \
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY"; \
}
LS_CONSTRUCT(difference,       geos_difference)
LS_CONSTRUCT(intersection,     geos_intersection)
LS_CONSTRUCT(union_op,         geos_union)
LS_CONSTRUCT(sym_difference,   geos_sym_difference)
#undef LS_CONSTRUCT

// Python: base.py::simplify:L469
template <typename T>
std::string LineString<T>::simplify(double tol) const {
    auto res = detail::geos_simplify(geos_linestring_.get(), tol);
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}

// Python: linestring.py::parallel_offset:L152
template <typename T>
std::string LineString<T>::parallel_offset(double distance, int quad_segs) const {
    auto res = detail::geos_parallel_offset(geos_linestring_.get(), distance, quad_segs);
    return res ? detail::geos_to_wkt(res.get()) : "GEOMETRYCOLLECTION EMPTY";
}

// Python: base.py::convex_hull:L567
template <typename T>
Polygon<double> LineString<T>::convex_hull() const {
    auto res = detail::geos_convex_hull(geos_linestring_.get());
    if (!res || res->isEmpty()) return Polygon<double>();
    auto* poly = dynamic_cast<geos::geom::Polygon*>(res.get());
    if (!poly) return Polygon<double>();
    auto* cs = poly->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty()) return Polygon<double>();
    size_t n = cs->getSize();
    std::vector<double> c(n*2);
    for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
    return Polygon<double>(c.data(), n, 2);
}

// Python: base.py::normalize:L663
template <typename T>
void LineString<T>::normalize() { geos_linestring_->normalize(); }

} // namespace geometry
} // namespace shapely
