// Python Source: shapely/geometry/polygon.py
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

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_LINESTRING_DEFINED
template <typename T> class LineString;
#endif
#ifndef SHAPELY_GEOMETRY_POINT_DEFINED
template <typename T> class Point;
#endif
#ifndef SHAPELY_GEOMETRY_LINEARRING_DEFINED
template <typename T> class LinearRing;
#endif

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
    double length() const;
    std::vector<double> bounds() const;

    // -- Topology --
    Polygon<double> intersection(const Polygon<double>& other) const;
    Point<double> centroid() const;
    Polygon<double> buffer(double distance) const;
    void normalize();

private:
    template <typename U> friend class Polygon;
    template <typename U> friend class LineString;
    template <typename U> friend class Point;
    template <typename U> friend class LinearRing;
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
#include "shapely/geometry/linearring.h"
#include "shapely/detail/geos_utils.h"

#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/util/TopologyException.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

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

// -- coords ------------------------------------------------------------------

template <typename T>
std::vector<std::tuple<T, T>> Polygon<T>::coords() const {
    std::vector<std::tuple<T, T>> r; r.reserve(rows_);
    for (size_t i = 0; i < rows_; ++i) r.emplace_back(coords_[i*cols_], coords_[i*cols_+1]);
    return r;
}

// -- exterior / interiors ----------------------------------------------------

template <typename T>
LinearRing<double> Polygon<T>::exterior() const {
    // Will be defined after linearring.h is included
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

// -- Area --------------------------------------------------------------------

template <typename T>
double Polygon<T>::area() const { return geos_polygon_->getArea(); }

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

// -- Predicates (macro) ------------------------------------------------------

#define POLY_PRED_IMPL(METHOD, GEOS_FN) \
template <typename T> template <typename U> bool Polygon<T>::METHOD(const Point<U>& o) const { return detail::GEOS_FN(geos_polygon_.get(), o.geos_point_.get()); } \
template <typename T> template <typename U> bool Polygon<T>::METHOD(const LineString<U>& o) const { return detail::GEOS_FN(geos_polygon_.get(), o.geos_linestring_.get()); } \
template <typename T> template <typename U> bool Polygon<T>::METHOD(const Polygon<U>& o) const { return detail::GEOS_FN(geos_polygon_.get(), o.geos_polygon_.get()); }

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

// equals_exact
template <typename T> template <typename U> bool Polygon<T>::equals_exact(const Point<U>& o, double tol) const { return detail::geos_equals_exact(geos_polygon_.get(), o.geos_point_.get(), tol); }
template <typename T> template <typename U> bool Polygon<T>::equals_exact(const LineString<U>& o, double tol) const { return detail::geos_equals_exact(geos_polygon_.get(), o.geos_linestring_.get(), tol); }
template <typename T> template <typename U> bool Polygon<T>::equals_exact(const Polygon<U>& o, double tol) const { return detail::geos_equals_exact(geos_polygon_.get(), o.geos_polygon_.get(), tol); }

// intersects
template <typename T> bool Polygon<T>::intersects(const Polygon& o) const { return geos_polygon_->intersects(o.geos_polygon_.get()); }
template <typename T> template <typename U> bool Polygon<T>::intersects(const LineString<U>& o) const { return geos_polygon_->intersects(o.geos_linestring_.get()); }
template <typename T> template <typename U> bool Polygon<T>::intersects(const Point<U>& o) const { return geos_polygon_->intersects(o.geos_point_.get()); }

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

// -- hausdorff_distance -----------------------------------------------------

template <typename T> template <typename U> double Polygon<T>::hausdorff_distance(const Point<U>& o) const { return detail::geos_hausdorff_distance(geos_polygon_.get(), o.geos_point_.get()); }
template <typename T> template <typename U> double Polygon<T>::hausdorff_distance(const LineString<U>& o) const { return detail::geos_hausdorff_distance(geos_polygon_.get(), o.geos_linestring_.get()); }
template <typename T> template <typename U> double Polygon<T>::hausdorff_distance(const Polygon<U>& o) const { return detail::geos_hausdorff_distance(geos_polygon_.get(), o.geos_polygon_.get()); }

// -- Accessors ---------------------------------------------------------------

template <typename T> std::string Polygon<T>::wkt() const { return detail::geos_to_wkt(geos_polygon_.get()); }
template <typename T> std::string Polygon<T>::wkb_hex() const { return detail::geos_to_wkb_hex(geos_polygon_.get()); }
template <typename T> std::string Polygon<T>::type() const { return "Polygon"; }
template <typename T> std::string Polygon<T>::geom_type() const { return detail::geos_geom_type(geos_polygon_.get()); }
template <typename T> bool        Polygon<T>::has_z() const { return detail::geos_has_z(geos_polygon_.get()); }

// -- Properties --------------------------------------------------------------

template <typename T> bool   Polygon<T>::is_empty() const { return detail::geos_is_empty(geos_polygon_.get()); }
template <typename T> bool   Polygon<T>::is_simple() const { return detail::geos_is_simple(geos_polygon_.get()); }
template <typename T> bool   Polygon<T>::is_valid() const { return geos_polygon_->isEmpty() ? false : geos_polygon_->isValid(); }
template <typename T> double Polygon<T>::length() const { return geos_polygon_->getLength(); }
template <typename T> std::vector<double> Polygon<T>::bounds() const { return detail::geos_bounds(geos_polygon_.get()); }

// -- intersection ------------------------------------------------------------

template <typename T>
Polygon<double> Polygon<T>::intersection(const Polygon<double>& other) const {
    if (geos_polygon_->isEmpty() || other.geos_polygon_->isEmpty()) return Polygon<double>();
    if (!geos_polygon_->isValid() || !other.geos_polygon_->isValid()) return Polygon<double>();
    auto inter = geos_polygon_->intersection(other.geos_polygon_.get());
    if (!inter || inter->isEmpty()) return Polygon<double>();
    const auto* geom = inter.get();
    if (geom->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
        auto* gp = dynamic_cast<const geos::geom::Polygon*>(geom);
        if (gp) {
            auto* cs = gp->getExteriorRing()->getCoordinatesRO();
            if (cs && !cs->isEmpty()) {
                size_t n = cs->getSize();
                std::vector<double> c(n*2);
                for (size_t i = 0; i < n; ++i) { c[i*2]=cs->getAt(i).x; c[i*2+1]=cs->getAt(i).y; }
                return Polygon<double>(c.data(), n, 2);
            }
        }
    }
    return Polygon<double>();
}

// -- centroid ----------------------------------------------------------------

template <typename T>
Point<double> Polygon<T>::centroid() const {
    auto c = geos_polygon_->getCentroid();
    if (!c) return Point<double>(0, 0);
    auto* coord = c->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

// -- buffer ------------------------------------------------------------------

template <typename T>
Polygon<double> Polygon<T>::buffer(double distance) const {
    if (geos_polygon_->isEmpty()) return Polygon<double>();
    if (!geos_polygon_->isValid() || !geos_polygon_->isSimple()) return Polygon<double>();
    auto buf = geos_polygon_->buffer(distance, 16);
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

// -- normalize ---------------------------------------------------------------

template <typename T>
void Polygon<T>::normalize() { geos_polygon_->normalize(); }

} // namespace geometry
} // namespace shapely
