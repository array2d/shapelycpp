// Python Source: shapely/affinity.py
// Line Range: L1-L200 (affine_transform, rotate, scale, skew, translate)
// Alignment: strict
// EXEMPTION: cpp_geos_wrapping
// Reason: C++ wraps GEOS coordinate-level transforms.

#pragma once

#include <memory>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/util/GeometryTransformer.h>
#include <cmath>

namespace shapely {
namespace ops {

// Python: shapely/affinity.py::translate:L62
inline std::unique_ptr<geos::geom::Geometry> translate(
    const geos::geom::Geometry* g, double xoff, double yoff) {
    if (!g) return nullptr;
    auto ret = g->clone();
    // Move coordinates
    for (size_t i = 0; i < ret->getNumGeometries(); ++i) {
        auto* sub = ret->getGeometryN(i);
        if (sub) {
            auto* cs = const_cast<geos::geom::Geometry*>(sub)->getCoordinates();
            if (cs) {
                for (size_t j = 0; j < cs->getSize(); ++j) {
                    cs->getAt(j).x += xoff;
                    cs->getAt(j).y += yoff;
                }
            }
        }
    }
    return std::unique_ptr<geos::geom::Geometry>(ret);
}

// Python: shapely/affinity.py::scale:L119
inline std::unique_ptr<geos::geom::Geometry> scale(
    const geos::geom::Geometry* g, double xfact, double yfact,
    double origin_x = 0.0, double origin_y = 0.0) {
    if (!g) return nullptr;
    auto ret = g->clone();
    for (size_t i = 0; i < ret->getNumGeometries(); ++i) {
        auto* sub = ret->getGeometryN(i);
        if (sub) {
            auto* coords = const_cast<geos::geom::Geometry*>(sub)->getCoordinates();
            if (coords) {
                for (size_t j = 0; j < coords->getSize(); ++j) {
                    auto& c = coords->getAt(j);
                    c.x = origin_x + (c.x - origin_x) * xfact;
                    c.y = origin_y + (c.y - origin_y) * yfact;
                }
            }
        }
    }
    return std::unique_ptr<geos::geom::Geometry>(ret);
}

// Python: shapely/affinity.py::rotate:L89
inline std::unique_ptr<geos::geom::Geometry> rotate(
    const geos::geom::Geometry* g, double angle,
    double origin_x = 0.0, double origin_y = 0.0) {
    if (!g) return nullptr;
    auto ret = g->clone();
    double cos_a = std::cos(angle), sin_a = std::sin(angle);
    for (size_t i = 0; i < ret->getNumGeometries(); ++i) {
        auto* sub = ret->getGeometryN(i);
        if (sub) {
            auto* coords = const_cast<geos::geom::Geometry*>(sub)->getCoordinates();
            if (coords) {
                for (size_t j = 0; j < coords->getSize(); ++j) {
                    auto& c = coords->getAt(j);
                    double dx = c.x - origin_x;
                    double dy = c.y - origin_y;
                    c.x = origin_x + dx * cos_a - dy * sin_a;
                    c.y = origin_y + dx * sin_a + dy * cos_a;
                }
            }
        }
    }
    return std::unique_ptr<geos::geom::Geometry>(ret);
}

} // namespace ops
} // namespace shapely
