// pybind11 module for shapelycpp testing.

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <cstring>
#include <tuple>
#include "shapely/geometry/point.h"
#include "shapely/geometry/linestring.h"
#include "shapely/geometry/polygon.h"
#include "shapely/geometry/linearring.h"
#include "shapely/ops/nearest_points.h"

namespace py = pybind11;
using namespace shapely::geometry;

// ==========================================================================
// Helpers
// ==========================================================================

namespace {

template <typename T>
py::array_t<T> coords_to_array(const py::list& py_coords) {
    py::ssize_t n = py::len(py_coords);
    auto arr = py::array_t<T>({n, static_cast<py::ssize_t>(2)});
    auto buf = arr.template request();
    T* ptr = static_cast<T*>(buf.ptr);
    for (py::ssize_t i = 0; i < n; ++i) {
        auto t = py_coords[i].cast<py::tuple>();
        ptr[i*2]   = t[0].cast<T>();
        ptr[i*2+1] = t[1].cast<T>();
    }
    return arr;
}

template <typename T>
py::array_t<T> native_to_array(const T* data, size_t rows, size_t cols) {
    auto result = py::array_t<T>({static_cast<py::ssize_t>(rows), static_cast<py::ssize_t>(cols)});
    std::memcpy(result.request().ptr, data, rows * cols * sizeof(T));
    return result;
}

} // anon namespace

// ==========================================================================
// Factory functions
// ==========================================================================

Point<double> make_point_f64(double x, double y) { return Point<double>(x, y); }
Point<float>  make_point_f32(float x, float y)  { return Point<float>(x, y); }

LineString<double> make_linestring_f64(const py::list& coords) {
    auto arr = coords_to_array<double>(coords);
    auto buf = arr.request();
    return LineString<double>(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
}
LineString<float> make_linestring_f32(const py::list& coords) {
    auto arr = coords_to_array<float>(coords);
    auto buf = arr.request();
    return LineString<float>(static_cast<const float*>(buf.ptr), buf.shape[0], buf.shape[1]);
}

Polygon<double> make_polygon_f64(const py::list& coords) {
    auto arr = coords_to_array<double>(coords);
    auto buf = arr.request();
    return Polygon<double>(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
}
Polygon<float> make_polygon_f32(const py::list& coords) {
    auto arr = coords_to_array<float>(coords);
    auto buf = arr.request();
    return Polygon<float>(static_cast<const float*>(buf.ptr), buf.shape[0], buf.shape[1]);
}

LinearRing<double> make_linearring_f64(const py::list& coords) {
    auto arr = coords_to_array<double>(coords);
    auto buf = arr.request();
    return LinearRing<double>(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
}

// ==========================================================================
// Cross-type wrappers
// ==========================================================================

// -- Point predicates --
#define DEF_PT_PRED(GT, OTHER, SUFFIX) \
bool pt_contains_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.contains(o); } \
bool pt_within_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.within(o); } \
bool pt_crosses_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.crosses(o); } \
bool pt_disjoint_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.disjoint(o); } \
bool pt_overlaps_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.overlaps(o); } \
bool pt_touches_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.touches(o); } \
bool pt_covers_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.covers(o); } \
bool pt_covered_by_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.covered_by(o); } \
bool pt_equals_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.equals(o); } \
bool pt_equals_exact_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o, double tol) { return s.equals_exact(o, tol); } \
bool pt_intersects_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.intersects(o); } \
std::string pt_relate_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.relate(o); } \
bool pt_relate_pattern_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o, const std::string& p) { return s.relate_pattern(o, p); } \
double pt_hausdorff_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.hausdorff_distance(o); } \
double pt_dist_##SUFFIX(const Point<GT>& s, const OTHER<GT>& o) { return s.distance(o); }

DEF_PT_PRED(double, Point, pt)
DEF_PT_PRED(double, LineString, ls)
DEF_PT_PRED(double, Polygon, poly)

// -- LineString predicates --
#define DEF_LS_PRED(GT, OTHER, SUFFIX) \
bool ls_contains_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.contains(o); } \
bool ls_within_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.within(o); } \
bool ls_crosses_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.crosses(o); } \
bool ls_disjoint_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.disjoint(o); } \
bool ls_overlaps_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.overlaps(o); } \
bool ls_touches_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.touches(o); } \
bool ls_covers_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.covers(o); } \
bool ls_covered_by_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.covered_by(o); } \
bool ls_equals_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.equals(o); } \
bool ls_equals_exact_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o, double tol) { return s.equals_exact(o, tol); } \
bool ls_intersects_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.intersects(o); } \
std::string ls_relate_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.relate(o); } \
bool ls_relate_pattern_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o, const std::string& p) { return s.relate_pattern(o, p); } \
double ls_hausdorff_##SUFFIX(const LineString<GT>& s, const OTHER<GT>& o) { return s.hausdorff_distance(o); }

DEF_LS_PRED(double, Point, pt)
DEF_LS_PRED(double, LineString, ls)
DEF_LS_PRED(double, Polygon, poly)

// -- Polygon predicates --
#define DEF_POLY_PRED(GT, OTHER, SUFFIX) \
bool poly_contains_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.contains(o); } \
bool poly_within_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.within(o); } \
bool poly_crosses_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.crosses(o); } \
bool poly_disjoint_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.disjoint(o); } \
bool poly_overlaps_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.overlaps(o); } \
bool poly_touches_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.touches(o); } \
bool poly_covers_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.covers(o); } \
bool poly_covered_by_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.covered_by(o); } \
bool poly_equals_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.equals(o); } \
bool poly_equals_exact_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o, double tol) { return s.equals_exact(o, tol); } \
bool poly_intersects_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.intersects(o); } \
std::string poly_relate_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.relate(o); } \
bool poly_relate_pattern_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o, const std::string& p) { return s.relate_pattern(o, p); } \
double poly_hausdorff_##SUFFIX(const Polygon<GT>& s, const OTHER<GT>& o) { return s.hausdorff_distance(o); }

DEF_POLY_PRED(double, Point, pt)
DEF_POLY_PRED(double, LineString, ls)
DEF_POLY_PRED(double, Polygon, poly)

// -- Legacy wrappers --
double dist_pt_ls(const Point<double>& p, const LineString<double>& l)   { return p.distance(l); }
double dist_pt_poly(const Point<double>& p, const Polygon<double>& poly) { return p.distance(poly); }
double dist_ls_poly(const LineString<double>& l, const Polygon<double>& poly) { return l.distance(poly); }
double dist_poly_ls(const Polygon<double>& poly, const LineString<double>& l)   { return poly.distance(l); }

bool intersects_ls_poly(const LineString<double>& l, const Polygon<double>& poly)  { return l.intersects(poly); }
bool intersects_poly_ls(const Polygon<double>& poly, const LineString<double>& l)  { return poly.intersects(l); }
bool intersects_poly_poly(const Polygon<double>& p1, const Polygon<double>& p2)    { return p1.intersects(p2); }

double project_ls_pt(const LineString<double>& l, const Point<double>& p) { return l.project(p); }
std::tuple<double,double> interpolate_ls(const LineString<double>& l, double d) {
    auto r = l.interpolate(d); return {r.x, r.y};
}

py::array_t<double> intersection_poly_poly(const Polygon<double>& p1, const Polygon<double>& p2) {
    auto inter = p1.intersection(p2);
    return native_to_array(inter.data(), inter.rows(), inter.cols());
}
double intersection_area_poly_poly(const Polygon<double>& p1, const Polygon<double>& p2) {
    auto inter = p1.intersection(p2);
    return inter.area();
}

std::tuple<double,double,double,double> nearest_poly_ls(const Polygon<double>& poly, const LineString<double>& ls) {
    return shapely::ops::nearest_points(poly, ls);
}
std::tuple<double,double,double,double> nearest_ls_pt(const LineString<double>& ls, const Point<double>& pt) {
    return shapely::ops::nearest_points(ls, pt);
}

std::tuple<double,double> centroid_point(const Point<double>& p) { auto r=p.centroid(); return {r.x,r.y}; }
std::tuple<double,double> centroid_ls(const LineString<double>& l) { auto r=l.centroid(); return {r.x,r.y}; }
std::tuple<double,double> centroid_poly(const Polygon<double>& p) { auto r=p.centroid(); return {r.x,r.y}; }

py::array_t<double> polygon_exterior(const Polygon<double>& p) {
    auto ext = p.exterior();
    return native_to_array(ext.data(), ext.rows(), ext.cols());
}

// ==========================================================================
// Binding macros
// ==========================================================================

// Bind cross-type predicates for a geometry type
#define BIND_GEOM_PREDS(PREFIX, SUFFIX) \
m.def(#PREFIX "_contains_" #SUFFIX, &PREFIX ## _contains_ ## SUFFIX); \
m.def(#PREFIX "_within_" #SUFFIX, &PREFIX ## _within_ ## SUFFIX); \
m.def(#PREFIX "_crosses_" #SUFFIX, &PREFIX ## _crosses_ ## SUFFIX); \
m.def(#PREFIX "_disjoint_" #SUFFIX, &PREFIX ## _disjoint_ ## SUFFIX); \
m.def(#PREFIX "_overlaps_" #SUFFIX, &PREFIX ## _overlaps_ ## SUFFIX); \
m.def(#PREFIX "_touches_" #SUFFIX, &PREFIX ## _touches_ ## SUFFIX); \
m.def(#PREFIX "_covers_" #SUFFIX, &PREFIX ## _covers_ ## SUFFIX); \
m.def(#PREFIX "_covered_by_" #SUFFIX, &PREFIX ## _covered_by_ ## SUFFIX); \
m.def(#PREFIX "_equals_" #SUFFIX, &PREFIX ## _equals_ ## SUFFIX); \
m.def(#PREFIX "_equals_exact_" #SUFFIX, &PREFIX ## _equals_exact_ ## SUFFIX, py::arg("self"), py::arg("other"), py::arg("tolerance")); \
m.def(#PREFIX "_intersects_" #SUFFIX, &PREFIX ## _intersects_ ## SUFFIX); \
m.def(#PREFIX "_relate_" #SUFFIX, &PREFIX ## _relate_ ## SUFFIX); \
m.def(#PREFIX "_relate_pattern_" #SUFFIX, &PREFIX ## _relate_pattern_ ## SUFFIX, py::arg("self"), py::arg("other"), py::arg("pattern")); \
m.def(#PREFIX "_hausdorff_distance_" #SUFFIX, &PREFIX ## _hausdorff_ ## SUFFIX);

// ==========================================================================
// Module
// ==========================================================================

PYBIND11_MODULE(shapelycpp, m) {
    m.doc() = "shapelycpp test module";

    // -- Factories --
    m.def("point",         &make_point_f64,       py::arg("x"), py::arg("y"));
    m.def("point_f32",     &make_point_f32,       py::arg("x"), py::arg("y"));
    m.def("linestring",    &make_linestring_f64,  py::arg("coords"));
    m.def("linestring_f32",&make_linestring_f32,  py::arg("coords"));
    m.def("polygon",       &make_polygon_f64,     py::arg("coords"));
    m.def("polygon_f32",   &make_polygon_f32,     py::arg("coords"));
    m.def("linearring",    &make_linearring_f64,  py::arg("coords"));

    // -- Point<double> --
    py::class_<Point<double>>(m, "Point")
        .def(py::init<double, double>(), py::arg("x"), py::arg("y"))
        .def_readonly("x", &Point<double>::x)
        .def_readonly("y", &Point<double>::y)
        .def("coords",      &Point<double>::coords)
        .def("xy",          &Point<double>::xy)
        .def("wkt",         &Point<double>::wkt)
        .def("wkb_hex",     &Point<double>::wkb_hex)
        .def("geom_type",   &Point<double>::geom_type)
        .def("type",        &Point<double>::type)
        .def("has_z",       &Point<double>::has_z)
        .def("is_empty",    &Point<double>::is_empty)
        .def("is_simple",   &Point<double>::is_simple)
        .def("is_valid",    &Point<double>::is_valid)
        .def("area",        &Point<double>::area)
        .def("length",      &Point<double>::length)
        .def("bounds",      &Point<double>::bounds)
        .def("buffer",      &Point<double>::buffer)
        .def("normalize",   &Point<double>::normalize)
        .def("distance", [](const Point<double>& self, const Point<double>& o) { return self.distance(o); });

    // -- Point<float> --
    py::class_<Point<float>>(m, "PointF32")
        .def(py::init<float, float>(), py::arg("x"), py::arg("y"))
        .def_readonly("x", &Point<float>::x)
        .def_readonly("y", &Point<float>::y)
        .def("buffer",   &Point<float>::buffer)
        .def("is_valid", &Point<float>::is_valid)
        .def("distance", [](const Point<float>& self, const Point<float>& o) { return self.distance(o); });

    // -- LineString<double> --
    py::class_<LineString<double>>(m, "LineString")
        .def(py::init([](const py::array_t<double>& arr) {
            auto buf = arr.request();
            return new LineString<double>(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("coords",    &LineString<double>::coords)
        .def("xy",        &LineString<double>::xy)
        .def("wkt",       &LineString<double>::wkt)
        .def("wkb_hex",   &LineString<double>::wkb_hex)
        .def("geom_type", &LineString<double>::geom_type)
        .def("type",      &LineString<double>::type)
        .def("has_z",     &LineString<double>::has_z)
        .def("is_empty",  &LineString<double>::is_empty)
        .def("is_simple", &LineString<double>::is_simple)
        .def("is_valid",  &LineString<double>::is_valid)
        .def("is_closed", &LineString<double>::is_closed)
        .def("is_ring",   &LineString<double>::is_ring)
        .def("area",      &LineString<double>::area)
        .def("length",    &LineString<double>::length)
        .def("bounds",    &LineString<double>::bounds)
        .def("buffer",    &LineString<double>::buffer)
        .def("normalize", &LineString<double>::normalize)
        .def("distance", [](const LineString<double>& self, const LineString<double>& o) { return self.distance(o); })
        .def("intersects", [](const LineString<double>& self, const LineString<double>& o) { return self.intersects(o); });

    // -- LineString<float> --
    py::class_<LineString<float>>(m, "LineStringF32")
        .def(py::init([](const py::array_t<float>& arr) {
            auto buf = arr.request();
            return new LineString<float>(static_cast<const float*>(buf.ptr), buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("length",  &LineString<float>::length)
        .def("buffer",  &LineString<float>::buffer)
        .def("distance", [](const LineString<float>& self, const LineString<float>& o) { return self.distance(o); });

    // -- Polygon<double> --
    py::class_<Polygon<double>>(m, "Polygon")
        .def(py::init([](const py::array_t<double>& arr) {
            auto buf = arr.request();
            return new Polygon<double>(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("coords",    &Polygon<double>::coords)
        .def("wkt",       &Polygon<double>::wkt)
        .def("wkb_hex",   &Polygon<double>::wkb_hex)
        .def("geom_type", &Polygon<double>::geom_type)
        .def("type",      &Polygon<double>::type)
        .def("has_z",     &Polygon<double>::has_z)
        .def("area",      &Polygon<double>::area)
        .def("length",    &Polygon<double>::length)
        .def("bounds",    &Polygon<double>::bounds)
        .def("is_empty",  &Polygon<double>::is_empty)
        .def("is_simple", &Polygon<double>::is_simple)
        .def("is_valid",  &Polygon<double>::is_valid)
        .def("buffer",    &Polygon<double>::buffer)
        .def("normalize", &Polygon<double>::normalize)
        .def("distance", [](const Polygon<double>& self, const Polygon<double>& o) { return self.distance(o); })
        .def("intersects", [](const Polygon<double>& self, const Polygon<double>& o) { return self.intersects(o); })
        .def_property_readonly("coords_arr", [](const Polygon<double>& poly) {
            return native_to_array(poly.data(), poly.rows(), poly.cols());
        });

    // -- Polygon<float> --
    py::class_<Polygon<float>>(m, "PolygonF32")
        .def(py::init([](const py::array_t<float>& arr) {
            auto buf = arr.request();
            return new Polygon<float>(static_cast<const float*>(buf.ptr), buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("area",     &Polygon<float>::area)
        .def("is_valid", &Polygon<float>::is_valid)
        .def("is_empty", &Polygon<float>::is_empty)
        .def("buffer",   &Polygon<float>::buffer)
        .def("distance", [](const Polygon<float>& self, const Polygon<float>& o) { return self.distance(o); })
        .def_property_readonly("coords_arr", [](const Polygon<float>& poly) {
            return native_to_array(poly.data(), poly.rows(), poly.cols());
        });

    // -- LinearRing<double> --
    py::class_<LinearRing<double>>(m, "LinearRing")
        .def(py::init([](const py::array_t<double>& arr) {
            auto buf = arr.request();
            return new LinearRing<double>(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("coords",    &LinearRing<double>::coords)
        .def("xy",        &LinearRing<double>::xy)
        .def("wkt",       &LinearRing<double>::wkt)
        .def("wkb_hex",   &LinearRing<double>::wkb_hex)
        .def("geom_type", &LinearRing<double>::geom_type)
        .def("type",      &LinearRing<double>::type)
        .def("has_z",     &LinearRing<double>::has_z)
        .def("is_empty",  &LinearRing<double>::is_empty)
        .def("is_simple", &LinearRing<double>::is_simple)
        .def("is_valid",  &LinearRing<double>::is_valid)
        .def("is_closed", &LinearRing<double>::is_closed)
        .def("is_ring",   &LinearRing<double>::is_ring)
        .def("is_ccw",    &LinearRing<double>::is_ccw)
        .def("area",      &LinearRing<double>::area)
        .def("length",    &LinearRing<double>::length)
        .def("bounds",    &LinearRing<double>::bounds)
        .def("normalize", &LinearRing<double>::normalize);

    // -- Cross-type predicates --
    BIND_GEOM_PREDS(pt, pt)
    BIND_GEOM_PREDS(pt, ls)
    BIND_GEOM_PREDS(pt, poly)
    BIND_GEOM_PREDS(ls, pt)
    BIND_GEOM_PREDS(ls, ls)
    BIND_GEOM_PREDS(ls, poly)
    BIND_GEOM_PREDS(poly, pt)
    BIND_GEOM_PREDS(poly, ls)
    BIND_GEOM_PREDS(poly, poly)

    // -- Legacy free functions --
    m.def("distance_point_linestring",   &dist_pt_ls);
    m.def("distance_point_polygon",      &dist_pt_poly);
    m.def("distance_linestring_polygon", &dist_ls_poly);
    m.def("distance_polygon_linestring", &dist_poly_ls);
    m.def("intersects_linestring_polygon", &intersects_ls_poly);
    m.def("intersects_polygon_linestring", &intersects_poly_ls);
    m.def("intersects_polygon_polygon",    &intersects_poly_poly);
    m.def("project_linestring_point",   &project_ls_pt);
    m.def("interpolate_linestring",     &interpolate_ls);
    m.def("intersection_polygon_polygon",      &intersection_poly_poly);
    m.def("intersection_area_polygon_polygon", &intersection_area_poly_poly);

    // -- Centroid --
    m.def("centroid_point",   &centroid_point);
    m.def("centroid_linestring", &centroid_ls);
    m.def("centroid_polygon",    &centroid_poly);

    // -- Polygon exterior --
    m.def("polygon_exterior", &polygon_exterior);

    // -- shapely.ops --
    m.def("nearest_points",      &nearest_poly_ls);
    m.def("nearest_points_ls_pt",&nearest_ls_pt);
}
