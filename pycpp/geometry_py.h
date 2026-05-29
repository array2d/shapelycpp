// Pybind11 wrappers for shapelycpp geometry types.
//
// Thin layer: accept Python types (py::array_t<T>, scalars) → construct C++
// geometry types → call native methods → wrap results back to Python.
//
// All factories are overloaded by coordinate type — no _f32 suffixes.
// linestring/polygon/linearring use py::array_t<T> so pybind11 distinguishes
// by array dtype.  point has both scalar (x,y) and array ([x,y]) forms.

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "../shapely/geometry/point.h"
#include "../shapely/geometry/linestring.h"
#include "../shapely/geometry/polygon.h"

#include <cstring>
#include <tuple>
#include <vector>

namespace py = pybind11;
using namespace shapely::geometry;

namespace shapely_py {

// ============================================================================
// Internal helpers
// ============================================================================

/// Copy native T* data to a Python numpy array.
template <typename T>
py::array_t<T> native_to_array(const T* data, size_t rows, size_t cols) {
    auto result = py::array_t<T>({static_cast<py::ssize_t>(rows),
                                   static_cast<py::ssize_t>(cols)});
    std::memcpy(result.request().ptr, data, rows * cols * sizeof(T));
    return result;
}

// ============================================================================
// Factory functions — all overloaded, no _f32 suffixes
// ============================================================================

// -- Point: scalar (x,y) + array ([x,y]) forms --
inline Point<double> point(double x, double y) { return Point<double>(x, y); }
inline Point<float>  point(float x, float y)  { return Point<float>(x, y); }

inline Point<double> point(const py::array_t<double>& arr) {
    auto buf = arr.request();
    const double* p = static_cast<const double*>(buf.ptr);
    return Point<double>(buf.size > 0 ? p[0] : 0, buf.size > 1 ? p[1] : 0);
}
inline Point<float> point(const py::array_t<float>& arr) {
    auto buf = arr.request();
    const float* p = static_cast<const float*>(buf.ptr);
    return Point<float>(buf.size > 0 ? p[0] : 0, buf.size > 1 ? p[1] : 0);
}

// -- LineString --
inline LineString<double> linestring(const py::array_t<double>& arr) {
    auto buf = arr.request();
    return LineString<double>(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
}
inline LineString<float> linestring(const py::array_t<float>& arr) {
    auto buf = arr.request();
    return LineString<float>(static_cast<const float*>(buf.ptr), buf.shape[0], buf.shape[1]);
}

// -- Polygon --
inline Polygon<double> polygon(const py::array_t<double>& arr) {
    auto buf = arr.request();
    return Polygon<double>(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
}
inline Polygon<float> polygon(const py::array_t<float>& arr) {
    auto buf = arr.request();
    return Polygon<float>(static_cast<const float*>(buf.ptr), buf.shape[0], buf.shape[1]);
}

// -- LinearRing (double only for now) --
inline LinearRing<double> linearring(const py::array_t<double>& arr) {
    auto buf = arr.request();
    return LinearRing<double>(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
}

// ============================================================================
// Cross-type distance
// ============================================================================

inline double distance_pt_ls(const Point<double>& p, const LineString<double>& l) { return p.distance(l); }
inline double distance_pt_poly(const Point<double>& p, const Polygon<double>& poly) { return p.distance(poly); }
inline double distance_ls_poly(const LineString<double>& l, const Polygon<double>& poly) { return l.distance(poly); }
inline double distance_poly_ls(const Polygon<double>& poly, const LineString<double>& l) { return poly.distance(l); }

// ============================================================================
// Cross-type intersects
// ============================================================================

inline bool intersects_ls_poly(const LineString<double>& l, const Polygon<double>& poly) { return l.intersects(poly); }
inline bool intersects_poly_ls(const Polygon<double>& poly, const LineString<double>& l) { return poly.intersects(l); }
inline bool intersects_poly_poly(const Polygon<double>& p1, const Polygon<double>& p2) { return p1.intersects(p2); }

// ============================================================================
// Predicates — all 9 cross-type pairs
// ============================================================================

// -- Point ↔ Point --
inline bool pt_contains_pt(const Point<double>& s, const Point<double>& o) { return s.contains(o); }
inline bool pt_within_pt(const Point<double>& s, const Point<double>& o) { return s.within(o); }
inline bool pt_disjoint_pt(const Point<double>& s, const Point<double>& o) { return s.disjoint(o); }
inline bool pt_touches_pt(const Point<double>& s, const Point<double>& o) { return s.touches(o); }
inline bool pt_crosses_pt(const Point<double>& s, const Point<double>& o) { return s.crosses(o); }
inline bool pt_overlaps_pt(const Point<double>& s, const Point<double>& o) { return s.overlaps(o); }
inline bool pt_covers_pt(const Point<double>& s, const Point<double>& o) { return s.covers(o); }
inline bool pt_covered_by_pt(const Point<double>& s, const Point<double>& o) { return s.covered_by(o); }
inline bool pt_equals_pt(const Point<double>& s, const Point<double>& o) { return s.equals(o); }
inline bool pt_equals_exact_pt(const Point<double>& s, const Point<double>& o, double tol) { return s.equals_exact(o, tol); }
inline bool pt_intersects_pt(const Point<double>& s, const Point<double>& o) { return s.intersects(o); }
inline std::string pt_relate_pt(const Point<double>& s, const Point<double>& o) { return s.relate(o); }
inline double pt_hausdorff_distance_pt(const Point<double>& s, const Point<double>& o) { return s.hausdorff_distance(o); }

// -- Point ↔ LineString --
inline bool pt_contains_ls(const Point<double>& s, const LineString<double>& o) { return s.contains(o); }
inline bool pt_within_ls(const Point<double>& s, const LineString<double>& o) { return s.within(o); }
inline bool pt_disjoint_ls(const Point<double>& s, const LineString<double>& o) { return s.disjoint(o); }
inline bool pt_touches_ls(const Point<double>& s, const LineString<double>& o) { return s.touches(o); }
inline bool pt_crosses_ls(const Point<double>& s, const LineString<double>& o) { return s.crosses(o); }
inline bool pt_overlaps_ls(const Point<double>& s, const LineString<double>& o) { return s.overlaps(o); }
inline bool pt_covers_ls(const Point<double>& s, const LineString<double>& o) { return s.covers(o); }
inline bool pt_covered_by_ls(const Point<double>& s, const LineString<double>& o) { return s.covered_by(o); }
inline bool pt_equals_ls(const Point<double>& s, const LineString<double>& o) { return s.equals(o); }
inline bool pt_equals_exact_ls(const Point<double>& s, const LineString<double>& o, double tol) { return s.equals_exact(o, tol); }
inline bool pt_intersects_ls(const Point<double>& s, const LineString<double>& o) { return s.intersects(o); }
inline std::string pt_relate_ls(const Point<double>& s, const LineString<double>& o) { return s.relate(o); }
inline double pt_hausdorff_distance_ls(const Point<double>& s, const LineString<double>& o) { return s.hausdorff_distance(o); }

// -- Point ↔ Polygon --
inline bool pt_contains_poly(const Point<double>& s, const Polygon<double>& o) { return s.contains(o); }
inline bool pt_within_poly(const Point<double>& s, const Polygon<double>& o) { return s.within(o); }
inline bool pt_disjoint_poly(const Point<double>& s, const Polygon<double>& o) { return s.disjoint(o); }
inline bool pt_touches_poly(const Point<double>& s, const Polygon<double>& o) { return s.touches(o); }
inline bool pt_crosses_poly(const Point<double>& s, const Polygon<double>& o) { return s.crosses(o); }
inline bool pt_overlaps_poly(const Point<double>& s, const Polygon<double>& o) { return s.overlaps(o); }
inline bool pt_covers_poly(const Point<double>& s, const Polygon<double>& o) { return s.covers(o); }
inline bool pt_covered_by_poly(const Point<double>& s, const Polygon<double>& o) { return s.covered_by(o); }
inline bool pt_equals_poly(const Point<double>& s, const Polygon<double>& o) { return s.equals(o); }
inline bool pt_equals_exact_poly(const Point<double>& s, const Polygon<double>& o, double tol) { return s.equals_exact(o, tol); }
inline bool pt_intersects_poly(const Point<double>& s, const Polygon<double>& o) { return s.intersects(o); }
inline std::string pt_relate_poly(const Point<double>& s, const Polygon<double>& o) { return s.relate(o); }
inline double pt_hausdorff_distance_poly(const Point<double>& s, const Polygon<double>& o) { return s.hausdorff_distance(o); }

// -- LineString ↔ Point --
inline bool ls_contains_pt(const LineString<double>& s, const Point<double>& o) { return s.contains(o); }
inline bool ls_within_pt(const LineString<double>& s, const Point<double>& o) { return s.within(o); }
inline bool ls_disjoint_pt(const LineString<double>& s, const Point<double>& o) { return s.disjoint(o); }
inline bool ls_touches_pt(const LineString<double>& s, const Point<double>& o) { return s.touches(o); }
inline bool ls_crosses_pt(const LineString<double>& s, const Point<double>& o) { return s.crosses(o); }
inline bool ls_overlaps_pt(const LineString<double>& s, const Point<double>& o) { return s.overlaps(o); }
inline bool ls_covers_pt(const LineString<double>& s, const Point<double>& o) { return s.covers(o); }
inline bool ls_covered_by_pt(const LineString<double>& s, const Point<double>& o) { return s.covered_by(o); }
inline bool ls_equals_pt(const LineString<double>& s, const Point<double>& o) { return s.equals(o); }
inline bool ls_equals_exact_pt(const LineString<double>& s, const Point<double>& o, double tol) { return s.equals_exact(o, tol); }
inline bool ls_intersects_pt(const LineString<double>& s, const Point<double>& o) { return s.intersects(o); }
inline std::string ls_relate_pt(const LineString<double>& s, const Point<double>& o) { return s.relate(o); }
inline double ls_hausdorff_distance_pt(const LineString<double>& s, const Point<double>& o) { return s.hausdorff_distance(o); }

// -- LineString ↔ LineString --
inline bool ls_contains_ls(const LineString<double>& s, const LineString<double>& o) { return s.contains(o); }
inline bool ls_within_ls(const LineString<double>& s, const LineString<double>& o) { return s.within(o); }
inline bool ls_disjoint_ls(const LineString<double>& s, const LineString<double>& o) { return s.disjoint(o); }
inline bool ls_touches_ls(const LineString<double>& s, const LineString<double>& o) { return s.touches(o); }
inline bool ls_crosses_ls(const LineString<double>& s, const LineString<double>& o) { return s.crosses(o); }
inline bool ls_overlaps_ls(const LineString<double>& s, const LineString<double>& o) { return s.overlaps(o); }
inline bool ls_covers_ls(const LineString<double>& s, const LineString<double>& o) { return s.covers(o); }
inline bool ls_covered_by_ls(const LineString<double>& s, const LineString<double>& o) { return s.covered_by(o); }
inline bool ls_equals_ls(const LineString<double>& s, const LineString<double>& o) { return s.equals(o); }
inline bool ls_equals_exact_ls(const LineString<double>& s, const LineString<double>& o, double tol) { return s.equals_exact(o, tol); }
inline bool ls_intersects_ls(const LineString<double>& s, const LineString<double>& o) { return s.intersects(o); }
inline std::string ls_relate_ls(const LineString<double>& s, const LineString<double>& o) { return s.relate(o); }
inline double ls_hausdorff_distance_ls(const LineString<double>& s, const LineString<double>& o) { return s.hausdorff_distance(o); }

// -- LineString ↔ Polygon --
inline bool ls_contains_poly(const LineString<double>& s, const Polygon<double>& o) { return s.contains(o); }
inline bool ls_within_poly(const LineString<double>& s, const Polygon<double>& o) { return s.within(o); }
inline bool ls_disjoint_poly(const LineString<double>& s, const Polygon<double>& o) { return s.disjoint(o); }
inline bool ls_touches_poly(const LineString<double>& s, const Polygon<double>& o) { return s.touches(o); }
inline bool ls_crosses_poly(const LineString<double>& s, const Polygon<double>& o) { return s.crosses(o); }
inline bool ls_overlaps_poly(const LineString<double>& s, const Polygon<double>& o) { return s.overlaps(o); }
inline bool ls_covers_poly(const LineString<double>& s, const Polygon<double>& o) { return s.covers(o); }
inline bool ls_covered_by_poly(const LineString<double>& s, const Polygon<double>& o) { return s.covered_by(o); }
inline bool ls_equals_poly(const LineString<double>& s, const Polygon<double>& o) { return s.equals(o); }
inline bool ls_equals_exact_poly(const LineString<double>& s, const Polygon<double>& o, double tol) { return s.equals_exact(o, tol); }
inline bool ls_intersects_poly(const LineString<double>& s, const Polygon<double>& o) { return s.intersects(o); }
inline std::string ls_relate_poly(const LineString<double>& s, const Polygon<double>& o) { return s.relate(o); }
inline double ls_hausdorff_distance_poly(const LineString<double>& s, const Polygon<double>& o) { return s.hausdorff_distance(o); }

// -- Polygon ↔ Point --
inline bool poly_contains_pt(const Polygon<double>& s, const Point<double>& o) { return s.contains(o); }
inline bool poly_within_pt(const Polygon<double>& s, const Point<double>& o) { return s.within(o); }
inline bool poly_disjoint_pt(const Polygon<double>& s, const Point<double>& o) { return s.disjoint(o); }
inline bool poly_touches_pt(const Polygon<double>& s, const Point<double>& o) { return s.touches(o); }
inline bool poly_crosses_pt(const Polygon<double>& s, const Point<double>& o) { return s.crosses(o); }
inline bool poly_overlaps_pt(const Polygon<double>& s, const Point<double>& o) { return s.overlaps(o); }
inline bool poly_covers_pt(const Polygon<double>& s, const Point<double>& o) { return s.covers(o); }
inline bool poly_covered_by_pt(const Polygon<double>& s, const Point<double>& o) { return s.covered_by(o); }
inline bool poly_equals_pt(const Polygon<double>& s, const Point<double>& o) { return s.equals(o); }
inline bool poly_equals_exact_pt(const Polygon<double>& s, const Point<double>& o, double tol) { return s.equals_exact(o, tol); }
inline bool poly_intersects_pt(const Polygon<double>& s, const Point<double>& o) { return s.intersects(o); }
inline std::string poly_relate_pt(const Polygon<double>& s, const Point<double>& o) { return s.relate(o); }
inline double poly_hausdorff_distance_pt(const Polygon<double>& s, const Point<double>& o) { return s.hausdorff_distance(o); }

// -- Polygon ↔ LineString --
inline bool poly_contains_ls(const Polygon<double>& s, const LineString<double>& o) { return s.contains(o); }
inline bool poly_within_ls(const Polygon<double>& s, const LineString<double>& o) { return s.within(o); }
inline bool poly_disjoint_ls(const Polygon<double>& s, const LineString<double>& o) { return s.disjoint(o); }
inline bool poly_touches_ls(const Polygon<double>& s, const LineString<double>& o) { return s.touches(o); }
inline bool poly_crosses_ls(const Polygon<double>& s, const LineString<double>& o) { return s.crosses(o); }
inline bool poly_overlaps_ls(const Polygon<double>& s, const LineString<double>& o) { return s.overlaps(o); }
inline bool poly_covers_ls(const Polygon<double>& s, const LineString<double>& o) { return s.covers(o); }
inline bool poly_covered_by_ls(const Polygon<double>& s, const LineString<double>& o) { return s.covered_by(o); }
inline bool poly_equals_ls(const Polygon<double>& s, const LineString<double>& o) { return s.equals(o); }
inline bool poly_equals_exact_ls(const Polygon<double>& s, const LineString<double>& o, double tol) { return s.equals_exact(o, tol); }
inline bool poly_intersects_ls(const Polygon<double>& s, const LineString<double>& o) { return s.intersects(o); }
inline std::string poly_relate_ls(const Polygon<double>& s, const LineString<double>& o) { return s.relate(o); }
inline double poly_hausdorff_distance_ls(const Polygon<double>& s, const LineString<double>& o) { return s.hausdorff_distance(o); }

// -- Polygon ↔ Polygon --
inline bool poly_contains_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.contains(o); }
inline bool poly_within_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.within(o); }
inline bool poly_disjoint_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.disjoint(o); }
inline bool poly_touches_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.touches(o); }
inline bool poly_crosses_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.crosses(o); }
inline bool poly_overlaps_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.overlaps(o); }
inline bool poly_covers_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.covers(o); }
inline bool poly_covered_by_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.covered_by(o); }
inline bool poly_equals_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.equals(o); }
inline bool poly_equals_exact_poly(const Polygon<double>& s, const Polygon<double>& o, double tol) { return s.equals_exact(o, tol); }
inline bool poly_intersects_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.intersects(o); }
inline std::string poly_relate_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.relate(o); }
inline double poly_hausdorff_distance_poly(const Polygon<double>& s, const Polygon<double>& o) { return s.hausdorff_distance(o); }

// ============================================================================
// Centroid, project, interpolate
// ============================================================================

inline std::tuple<double,double> centroid_point(const Point<double>& p) { auto r=p.centroid(); return {r.x,r.y}; }
inline std::tuple<double,double> centroid_linestring(const LineString<double>& l) { auto r=l.centroid(); return {r.x,r.y}; }
inline std::tuple<double,double> centroid_polygon(const Polygon<double>& p) { auto r=p.centroid(); return {r.x,r.y}; }

inline double project_ls_pt(const LineString<double>& l, const Point<double>& p) { return l.project(p); }
inline std::tuple<double,double> interpolate_ls(const LineString<double>& l, double d) { auto r=l.interpolate(d); return {r.x,r.y}; }

inline double intersection_area_poly_poly(const Polygon<double>& p1, const Polygon<double>& p2) {
    return p1.intersection(p2).area();
}

inline py::array_t<double> polygon_exterior(const Polygon<double>& p) {
    auto ext = p.exterior();
    return native_to_array(ext.data(), ext.rows(), ext.cols());
}

} // namespace shapely_py

// ============================================================================
// Pybind11 binding helper macros
// ============================================================================

#define BIND_PREDS(m, SRC, TGT) \
    m.def(#SRC "_contains_" #TGT, &shapely_py::SRC ## _contains_ ## TGT); \
    m.def(#SRC "_within_" #TGT, &shapely_py::SRC ## _within_ ## TGT); \
    m.def(#SRC "_crosses_" #TGT, &shapely_py::SRC ## _crosses_ ## TGT); \
    m.def(#SRC "_disjoint_" #TGT, &shapely_py::SRC ## _disjoint_ ## TGT); \
    m.def(#SRC "_overlaps_" #TGT, &shapely_py::SRC ## _overlaps_ ## TGT); \
    m.def(#SRC "_touches_" #TGT, &shapely_py::SRC ## _touches_ ## TGT); \
    m.def(#SRC "_covers_" #TGT, &shapely_py::SRC ## _covers_ ## TGT); \
    m.def(#SRC "_covered_by_" #TGT, &shapely_py::SRC ## _covered_by_ ## TGT); \
    m.def(#SRC "_equals_" #TGT, &shapely_py::SRC ## _equals_ ## TGT); \
    m.def(#SRC "_equals_exact_" #TGT, &shapely_py::SRC ## _equals_exact_ ## TGT, \
          py::arg("self"), py::arg("other"), py::arg("tolerance")); \
    m.def(#SRC "_intersects_" #TGT, &shapely_py::SRC ## _intersects_ ## TGT); \
    m.def(#SRC "_relate_" #TGT, &shapely_py::SRC ## _relate_ ## TGT); \
    m.def(#SRC "_hausdorff_distance_" #TGT, &shapely_py::SRC ## _hausdorff_distance_ ## TGT);

#define BIND_ACCESSORS(CLS) \
    .def("wkt", &CLS::wkt) \
    .def("wkb_hex", &CLS::wkb_hex) \
    .def("geom_type", &CLS::geom_type) \
    .def("type", &CLS::type) \
    .def("has_z", &CLS::has_z) \
    .def("is_empty", &CLS::is_empty) \
    .def("is_simple", &CLS::is_simple) \
    .def("is_valid", &CLS::is_valid) \
    .def("area", &CLS::area) \
    .def("length", &CLS::length) \
    .def("bounds", &CLS::bounds) \
    .def("normalize", &CLS::normalize)
