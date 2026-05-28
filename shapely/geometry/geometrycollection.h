// Python Source: shapely/geometry/collection.py
// Line Range: L10-L60 (class GeometryCollection)
// Alignment: strict
// EXEMPTION: cpp_non_template
// Reason: GeometryCollection can hold mixed types; non-template for simplicity.

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstddef>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Geometry.h>

namespace shapely {
namespace geometry {

class GeometryCollection {
public:
    GeometryCollection();

    /// Add any GEOS geometry (advanced usage)
    void add_geometry(std::unique_ptr<geos::geom::Geometry> geom);

    GeometryCollection(GeometryCollection&&) = default;
    GeometryCollection& operator=(GeometryCollection&&) = default;

    // -- Access to individual geometries --
    size_t num_geometries() const;
    const geos::geom::Geometry* geometry_n(size_t i) const;

    // -- Predicates with another GeometryCollection --
    bool contains(const GeometryCollection& other) const;
    bool within(const GeometryCollection& other) const;
    bool disjoint(const GeometryCollection& other) const;
    bool overlaps(const GeometryCollection& other) const;
    bool touches(const GeometryCollection& other) const;
    bool equals(const GeometryCollection& other) const;
    bool equals_exact(const GeometryCollection& other, double tol) const;
    bool intersects(const GeometryCollection& other) const;
    std::string relate(const GeometryCollection& other) const;
    bool relate_pattern(const GeometryCollection& other, const std::string& p) const;
    double hausdorff_distance(const GeometryCollection& other) const;

    // -- Constructive operations --
    GeometryCollection difference(const GeometryCollection& other) const;
    GeometryCollection intersection(const GeometryCollection& other) const;
    GeometryCollection union_op(const GeometryCollection& other) const;
    GeometryCollection sym_difference(const GeometryCollection& other) const;
    GeometryCollection simplify(double tolerance) const;

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
    GeometryCollection convex_hull() const;
    GeometryCollection buffer(double distance) const;
    void normalize();

    // -- Access to internal GEOS geometry (for interop) --
    const geos::geom::GeometryCollection* geos_collection() const { return geos_coll_.get(); }

private:
    std::unique_ptr<geos::geom::GeometryCollection> geos_coll_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

// ============================================================================
// Implementation
// ============================================================================

#include "shapely/geometry/base.h"
#include <geos/geom/Coordinate.h>
#include <geos/operation/distance/DistanceOp.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

GeometryCollection::GeometryCollection() {
    factory_ = geos::geom::GeometryFactory::create();
    geos_coll_ = factory_->createGeometryCollection();
}

void GeometryCollection::add_geometry(std::unique_ptr<geos::geom::Geometry> geom) {
    if (!geom) return;
    std::vector<std::unique_ptr<geos::geom::Geometry>> geoms;
    for (size_t i = 0; i < geos_coll_->getNumGeometries(); ++i)
        geoms.push_back(std::unique_ptr<geos::geom::Geometry>(geos_coll_->getGeometryN(i)->clone()));
    geoms.push_back(std::move(geom));
    geos_coll_ = factory_->createGeometryCollection(std::move(geoms));
}

size_t GeometryCollection::num_geometries() const { return geos_coll_->getNumGeometries(); }

const geos::geom::Geometry* GeometryCollection::geometry_n(size_t i) const {
    return geos_coll_->getGeometryN(i);
}

// -- Predicates (delegate to GEOS) -------------------------------------------
#define GC_PRED(METHOD, GEOS_FN) \
bool GeometryCollection::METHOD(const GeometryCollection& o) const { return detail::GEOS_FN(geos_coll_.get(), o.geos_coll_.get()); }
GC_PRED(contains,    geos_contains)
GC_PRED(within,      geos_within)
GC_PRED(disjoint,    geos_disjoint)
GC_PRED(overlaps,    geos_overlaps)
GC_PRED(touches,     geos_touches)
GC_PRED(equals,      geos_equals)
#undef GC_PRED

bool GeometryCollection::equals_exact(const GeometryCollection& o, double tol) const {
    return detail::geos_equals_exact(geos_coll_.get(), o.geos_coll_.get(), tol);
}
bool GeometryCollection::intersects(const GeometryCollection& o) const {
    return geos_coll_->intersects(o.geos_coll_.get());
}
std::string GeometryCollection::relate(const GeometryCollection& o) const {
    return detail::geos_relate(geos_coll_.get(), o.geos_coll_.get());
}
bool GeometryCollection::relate_pattern(const GeometryCollection& o, const std::string& p) const {
    return detail::geos_relate_pattern(geos_coll_.get(), o.geos_coll_.get(), p);
}
double GeometryCollection::hausdorff_distance(const GeometryCollection& o) const {
    return detail::geos_hausdorff_distance(geos_coll_.get(), o.geos_coll_.get());
}

// -- Constructive operations -----------------------------------------------
#define GC_CONSTRUCT(OP, GEOS_FN) \
GeometryCollection GeometryCollection::OP(const GeometryCollection& o) const { \
    auto res = detail::GEOS_FN(geos_coll_.get(), o.geos_coll_.get()); \
    GeometryCollection r; \
    if (res && !res->isEmpty()) { \
        for (size_t i = 0; i < res->getNumGeometries(); ++i) \
            r.add_geometry(std::unique_ptr<geos::geom::Geometry>(res->getGeometryN(i)->clone())); \
    } \
    return r; \
}
GC_CONSTRUCT(difference,       geos_difference)
GC_CONSTRUCT(intersection,     geos_intersection)
GC_CONSTRUCT(union_op,         geos_union)
GC_CONSTRUCT(sym_difference,   geos_sym_difference)
#undef GC_CONSTRUCT

GeometryCollection GeometryCollection::simplify(double tol) const {
    auto res = detail::geos_simplify(geos_coll_.get(), tol);
    GeometryCollection r;
    if (res && !res->isEmpty()) {
        for (size_t i = 0; i < res->getNumGeometries(); ++i)
            r.add_geometry(std::unique_ptr<geos::geom::Geometry>(res->getGeometryN(i)->clone()));
    }
    return r;
}

// -- Accessors ---------------------------------------------------------------
std::string GeometryCollection::wkt() const { return detail::geos_to_wkt(geos_coll_.get()); }
std::string GeometryCollection::wkb_hex() const { return detail::geos_to_wkb_hex(geos_coll_.get()); }
std::string GeometryCollection::type() const { return "GeometryCollection"; }
std::string GeometryCollection::geom_type() const { return detail::geos_geom_type(geos_coll_.get()); }
bool GeometryCollection::has_z() const { return detail::geos_has_z(geos_coll_.get()); }

// -- Properties ---------------------------------------------------------------
bool GeometryCollection::is_empty() const { return detail::geos_is_empty(geos_coll_.get()); }
bool GeometryCollection::is_simple() const { return detail::geos_is_simple(geos_coll_.get()); }
bool GeometryCollection::is_valid() const { return detail::geos_is_valid(geos_coll_.get()); }
double GeometryCollection::area() const { return geos_coll_->getArea(); }
double GeometryCollection::length() const { return geos_coll_->getLength(); }
std::vector<double> GeometryCollection::bounds() const { return detail::geos_bounds(geos_coll_.get()); }

// -- Topology ------------------------------------------------------------------
GeometryCollection GeometryCollection::convex_hull() const {
    auto res = detail::geos_convex_hull(geos_coll_.get());
    GeometryCollection r;
    if (res && !res->isEmpty()) {
        for (size_t i = 0; i < res->getNumGeometries(); ++i)
            r.add_geometry(std::unique_ptr<geos::geom::Geometry>(res->getGeometryN(i)->clone()));
    }
    return r;
}

GeometryCollection GeometryCollection::buffer(double distance) const {
    auto buf = geos_coll_->buffer(distance, 16);
    GeometryCollection r;
    if (buf && !buf->isEmpty()) {
        for (size_t i = 0; i < buf->getNumGeometries(); ++i)
            r.add_geometry(std::unique_ptr<geos::geom::Geometry>(buf->getGeometryN(i)->clone()));
    }
    return r;
}

void GeometryCollection::normalize() { geos_coll_->normalize(); }

} // namespace geometry
} // namespace shapely
