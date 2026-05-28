// Internal GEOS utility functions shared across geometry types.
// NOT part of public API — do not include directly.

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/io/WKTWriter.h>
#include <geos/io/WKBWriter.h>
#include <geos/algorithm/distance/DiscreteHausdorffDistance.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/operation/valid/IsValidOp.h>

namespace shapely {
namespace detail {

// --- WKT / WKB serialization ------------------------------------------------

inline std::string geos_to_wkt(const geos::geom::Geometry* g) {
    if (!g || g->isEmpty()) return "GEOMETRYCOLLECTION EMPTY";
    geos::io::WKTWriter writer;
    return writer.write(g);
}

inline std::string geos_to_wkb_hex(const geos::geom::Geometry* g) {
    if (!g || g->isEmpty()) return "";
    geos::io::WKBWriter writer;
    std::ostringstream oss;
    writer.writeHEX(*g, oss);
    return oss.str();
}

// --- Predicates -------------------------------------------------------------

inline bool geos_contains(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->contains(b);
}

inline bool geos_within(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->within(b);
}

inline bool geos_crosses(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->crosses(b);
}

inline bool geos_disjoint(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return true;
    return a->disjoint(b);
}

inline bool geos_overlaps(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->overlaps(b);
}

inline bool geos_touches(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->touches(b);
}

inline bool geos_covers(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->covers(b);
}

inline bool geos_covered_by(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->coveredBy(b);
}

inline bool geos_equals(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    return a->equals(b);
}

inline bool geos_equals_exact(const geos::geom::Geometry* a, const geos::geom::Geometry* b, double tol) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    return a->equalsExact(b, tol);
}

inline std::string geos_relate(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return "";
    return a->relate(b)->toString();
}

inline bool geos_relate_pattern(const geos::geom::Geometry* a, const geos::geom::Geometry* b, const std::string& pat) {
    if (!a || !b) return false;
    return a->relate(b, pat);
}

// --- Geometry property helpers ----------------------------------------------

inline std::string geos_geom_type(const geos::geom::Geometry* g) {
    if (!g) return "GeometryCollection";
    return g->getGeometryType();
}

inline bool geos_is_simple(const geos::geom::Geometry* g) {
    return g && g->isSimple();
}

inline bool geos_is_valid(const geos::geom::Geometry* g) {
    return g && g->isValid();
}

inline bool geos_is_empty(const geos::geom::Geometry* g) {
    return !g || g->isEmpty();
}

inline int geos_has_z(const geos::geom::Geometry* g) {
    return g ? (g->getCoordinateDimension() > 2) : 0;
}

inline double geos_area(const geos::geom::Geometry* g) {
    return g ? g->getArea() : 0.0;
}

inline double geos_length(const geos::geom::Geometry* g) {
    return g ? g->getLength() : 0.0;
}

inline double geos_hausdorff_distance(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return 0.0;
    return geos::algorithm::distance::DiscreteHausdorffDistance(*a, *b).distance();
}

inline std::vector<double> geos_bounds(const geos::geom::Geometry* g) {
    if (!g || g->isEmpty()) return {0, 0, 0, 0};
    const geos::geom::Envelope* env = g->getEnvelopeInternal();
    return {env->getMinX(), env->getMinY(), env->getMaxX(), env->getMaxY()};
}

} // namespace detail
} // namespace shapely
