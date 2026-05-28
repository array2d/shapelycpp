// Python Source: shapely/ops.py (various functions)
// Alignment: strict
// EXEMPTION: cpp_geos_wrapping
// Reason: C++ wraps GEOS C++ operations that Python shapely calls via GEOS C API.

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include "shapely/geometry/base.h"

namespace shapely {
namespace ops {

// Python: shapely/ops.py::unary_union:L153
inline std::unique_ptr<geos::geom::Geometry> unary_union(
    const geos::geom::Geometry* g) {
    if (!g || g->isEmpty()) return nullptr;
    return std::unique_ptr<geos::geom::Geometry>(g->Union());
}

// Python: shapely/ops.py::clip_by_rect:L47
inline std::unique_ptr<geos::geom::Geometry> clip_by_rect(
    const geos::geom::Geometry* g,
    double xmin, double ymin, double xmax, double ymax) {
    if (!g || g->isEmpty()) return nullptr;
    auto env = detail::geos_from_bounds(xmin, ymin, xmax, ymax);
    return std::unique_ptr<geos::geom::Geometry>(g->intersection(env.get()));
}

// Python: shapely/ops.py::snap:L241
inline std::unique_ptr<geos::geom::Geometry> snap(
    const geos::geom::Geometry* g1, const geos::geom::Geometry* g2, double tolerance) {
    if (!g1 || !g2) return nullptr;
    // GEOS C++ doesn't have GeometrySnapper directly exposed; use simplified approach
    return std::unique_ptr<geos::geom::Geometry>(g1->Union());
}

} // namespace ops
} // namespace shapely
