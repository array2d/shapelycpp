// Python Source: shapely/geometry/base.py
// Line Range: L146-L931 (class BaseGeometry + BaseMultipartGeometry)
// Alignment: strict
// EXEMPTION: cpp_internal_detail
// Reason: C++ implements Python BaseGeometry methods as internal detail:: helpers.
//         Python shapely calls GEOS C API directly in BaseGeometry;
//         C++ wraps GEOS C++ API in shapely::detail namespace.
//         These helpers are not part of the public shapely::geometry API.

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

// Python: shapely/geometry/base.py::wkt:L369
inline std::string geos_to_wkt(const geos::geom::Geometry* g) {
    if (!g || g->isEmpty()) return "GEOMETRYCOLLECTION EMPTY";
    geos::io::WKTWriter writer;
    return writer.write(g);
}

// Python: shapely/geometry/base.py::wkb_hex:L379
inline std::string geos_to_wkb_hex(const geos::geom::Geometry* g) {
    if (!g || g->isEmpty()) return "";
    geos::io::WKBWriter writer;
    std::ostringstream oss;
    writer.writeHEX(*g, oss);
    return oss.str();
}

// --- Predicates -------------------------------------------------------------

// Python: shapely/geometry/base.py::contains:L766
inline bool geos_contains(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->contains(b);
}

// Python: shapely/geometry/base.py::within:L813
inline bool geos_within(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->within(b);
}

// Python: shapely/geometry/base.py::crosses:L770
inline bool geos_crosses(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->crosses(b);
}

// Python: shapely/geometry/base.py::disjoint:L774
inline bool geos_disjoint(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return true;
    return a->disjoint(b);
}

// Python: shapely/geometry/base.py::overlaps:L805
inline bool geos_overlaps(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->overlaps(b);
}

// Python: shapely/geometry/base.py::touches:L809
inline bool geos_touches(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->touches(b);
}

// Python: shapely/geometry/base.py::covers:L758
inline bool geos_covers(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->covers(b);
}

// Python: shapely/geometry/base.py::covered_by:L762
inline bool geos_covered_by(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return false;
    return a->coveredBy(b);
}

// Python: shapely/geometry/base.py::equals:L778
inline bool geos_equals(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    return a->equals(b);
}

// Python: shapely/geometry/base.py::equals_exact:L817
inline bool geos_equals_exact(const geos::geom::Geometry* a, const geos::geom::Geometry* b, double tol) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    return a->equalsExact(b, tol);
}

// Python: shapely/geometry/base.py::relate:L753
inline std::string geos_relate(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return "";
    return a->relate(b)->toString();
}

// Python: shapely/geometry/base.py::relate_pattern:L890
inline bool geos_relate_pattern(const geos::geom::Geometry* a, const geos::geom::Geometry* b, const std::string& pat) {
    if (!a || !b) return false;
    return a->relate(b, pat);
}

// --- Geometry property helpers ----------------------------------------------

// Python: shapely/geometry/base.py::geom_type:L426
inline std::string geos_geom_type(const geos::geom::Geometry* g) {
    if (!g) return "GeometryCollection";
    return g->getGeometryType();
}

// Python: shapely/geometry/base.py::is_simple:L739
inline bool geos_is_simple(const geos::geom::Geometry* g) {
    return g && g->isSimple();
}

// Python: shapely/geometry/base.py::is_valid:L745
inline bool geos_is_valid(const geos::geom::Geometry* g) {
    return g && g->isValid();
}

// Python: shapely/geometry/base.py::is_empty:L714
inline bool geos_is_empty(const geos::geom::Geometry* g) {
    return !g || g->isEmpty();
}

// Python: shapely/geometry/base.py::has_z:L708
inline int geos_has_z(const geos::geom::Geometry* g) {
    return g ? (g->getCoordinateDimension() > 2) : 0;
}

// Python: shapely/geometry/base.py::area:L434
inline double geos_area(const geos::geom::Geometry* g) {
    return g ? g->getArea() : 0.0;
}

// Python: shapely/geometry/base.py::length:L447
inline double geos_length(const geos::geom::Geometry* g) {
    return g ? g->getLength() : 0.0;
}

// Python: shapely/geometry/base.py::hausdorff_distance:L442
inline double geos_hausdorff_distance(const geos::geom::Geometry* a, const geos::geom::Geometry* b) {
    if (!a || !b) return 0.0;
    return geos::algorithm::distance::DiscreteHausdorffDistance(*a, *b).distance();
}

// Python: shapely/geometry/base.py::bounds:L470
inline std::vector<double> geos_bounds(const geos::geom::Geometry* g) {
    if (!g || g->isEmpty()) return {0, 0, 0, 0};
    const geos::geom::Envelope* env = g->getEnvelopeInternal();
    return {env->getMinX(), env->getMinY(), env->getMaxX(), env->getMaxY()};
}

} // namespace detail
} // namespace shapely
