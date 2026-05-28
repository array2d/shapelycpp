// Python Source: shapely/wkt.py
// Line Range: L340-L420 (load, loads, dump, dumps)
// Alignment: strict

#pragma once

#include <string>
#include <memory>
#include <geos/geom/Geometry.h>
#include "shapely/geometry/base.h"

namespace shapely {
namespace io {

// Python: shapely/wkt.py::loads:L348
inline std::unique_ptr<geos::geom::Geometry> wkt_loads(const std::string& wkt) {
    return detail::geos_wkt_loads(wkt);
}

// Python: shapely/wkt.py::dumps:L395
inline std::string wkt_dumps(const geos::geom::Geometry* g) {
    return detail::geos_to_wkt(g);
}

} // namespace io
} // namespace shapely
