// Python Source: shapely/wkb.py
// Line Range: L240-L420 (load, loads, dump, dumps)
// Alignment: strict

#pragma once

#include <string>
#include <memory>
#include <geos/geom/Geometry.h>
#include "shapely/geometry/base.h"

namespace shapely {
namespace io {

// Python: shapely/wkb.py::loads (hex)
inline std::unique_ptr<geos::geom::Geometry> wkb_loads_hex(const std::string& hex) {
    return detail::geos_wkb_loads_hex(hex);
}

// Python: shapely/wkb.py::dumps (hex)
inline std::string wkb_dumps_hex(const geos::geom::Geometry* g) {
    return detail::geos_to_wkb_hex(g);
}

} // namespace io
} // namespace shapely
