// pybind11 module for shapelycpp testing.
// Wraps native shapely::geometry types via thin pycpp factories.
//
// NOTE: C++ 特有文件，Python 端无直接对应。
// This is a C++-only test module. Python shapely is the baseline.
// Each C++ function is named to match the Python shapely API exactly.

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <cstring>
#include "shapely/geometry/point.h"
#include "shapely/geometry/linestring.h"
#include "shapely/geometry/polygon.h"
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
    auto result = py::array_t<T>(
        {static_cast<py::ssize_t>(rows),
         static_cast<py::ssize_t>(cols)});
    std::memcpy(result.request().ptr, data, rows * cols * sizeof(T));
    return result;
}

} // anon namespace

// ==========================================================================
// Factory functions — py::array_t → native types
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

// ==========================================================================
// Cross-type wrappers
// ==========================================================================

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

// ops::nearest_points
std::tuple<double,double,double,double> nearest_poly_ls(const Polygon<double>& poly, const LineString<double>& ls) {
    return shapely::ops::nearest_points(poly, ls);
}
std::tuple<double,double,double,double> nearest_ls_pt(const LineString<double>& ls, const Point<double>& pt) {
    return shapely::ops::nearest_points(ls, pt);
}

// ==========================================================================
// Module
// ==========================================================================

PYBIND11_MODULE(shapelycpp, m) {
    m.doc() = "shapelycpp test module — C++ alignment of Python shapely, powered by GEOS";

    // ---- Factory functions --------------------------------------------------
    m.def("point",         &make_point_f64,       py::arg("x"), py::arg("y"));
    m.def("point_f32",     &make_point_f32,       py::arg("x"), py::arg("y"));
    m.def("linestring",    &make_linestring_f64,  py::arg("coords"));
    m.def("linestring_f32",&make_linestring_f32,  py::arg("coords"));
    m.def("polygon",       &make_polygon_f64,     py::arg("coords"));
    m.def("polygon_f32",   &make_polygon_f32,     py::arg("coords"));

    // ---- Point<double> -----------------------------------
    py::class_<Point<double>>(m, "Point")
        .def(py::init<double, double>(), py::arg("x"), py::arg("y"))
        .def("distance", [](const Point<double>& self, const Point<double>& other) { return self.distance(other); })
        .def("buffer",   &Point<double>::buffer)
        .def_readonly("x", &Point<double>::x)
        .def_readonly("y", &Point<double>::y);

    // ---- Point<float> ------------------------------------
    py::class_<Point<float>>(m, "PointF32")
        .def(py::init<float, float>(), py::arg("x"), py::arg("y"))
        .def("distance", [](const Point<float>& self, const Point<float>& other) { return self.distance(other); })
        .def("buffer",   &Point<float>::buffer)
        .def_readonly("x", &Point<float>::x)
        .def_readonly("y", &Point<float>::y);

    // ---- LineString<double> ------------------------------
    py::class_<LineString<double>>(m, "LineString")
        .def(py::init([](const py::array_t<double>& arr) {
            auto buf = arr.request();
            return new LineString<double>(
                static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("distance", [](const LineString<double>& self, const LineString<double>& other) { return self.distance(other); })
        .def("length",   &LineString<double>::length)
        .def("buffer",   &LineString<double>::buffer);

    // ---- LineString<float> -------------------------------
    py::class_<LineString<float>>(m, "LineStringF32")
        .def(py::init([](const py::array_t<float>& arr) {
            auto buf = arr.request();
            return new LineString<float>(
                static_cast<const float*>(buf.ptr), buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("distance", [](const LineString<float>& self, const LineString<float>& other) { return self.distance(other); })
        .def("length",   &LineString<float>::length)
        .def("buffer",   &LineString<float>::buffer);

    // ---- Polygon<double> ----------------------------------
    py::class_<Polygon<double>>(m, "Polygon")
        .def(py::init([](const py::array_t<double>& arr) {
            auto buf = arr.request();
            return new Polygon<double>(
                static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("distance", [](const Polygon<double>& self, const Polygon<double>& other) { return self.distance(other); })
        .def("area",     &Polygon<double>::area)
        .def("is_valid", &Polygon<double>::is_valid)
        .def("is_empty", &Polygon<double>::is_empty)
        .def("buffer",   &Polygon<double>::buffer)
        .def_property_readonly("coords", [](const Polygon<double>& poly) {
            return native_to_array(poly.data(), poly.rows(), poly.cols());
        });

    // ---- Polygon<float> -----------------------------------
    py::class_<Polygon<float>>(m, "PolygonF32")
        .def(py::init([](const py::array_t<float>& arr) {
            auto buf = arr.request();
            return new Polygon<float>(
                static_cast<const float*>(buf.ptr), buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("distance", [](const Polygon<float>& self, const Polygon<float>& other) { return self.distance(other); })
        .def("area",     &Polygon<float>::area)
        .def("is_valid", &Polygon<float>::is_valid)
        .def("is_empty", &Polygon<float>::is_empty)
        .def("buffer",   &Polygon<float>::buffer)
        .def_property_readonly("coords", [](const Polygon<float>& poly) {
            return native_to_array(poly.data(), poly.rows(), poly.cols());
        });

    // ---- Free functions (cross-type, ops) -----------------
    m.def("distance_point_linestring",   &dist_pt_ls,          py::arg("point"), py::arg("linestring"));
    m.def("distance_point_polygon",      &dist_pt_poly,        py::arg("point"), py::arg("polygon"));
    m.def("distance_linestring_polygon", &dist_ls_poly,        py::arg("linestring"), py::arg("polygon"));
    m.def("distance_polygon_linestring", &dist_poly_ls,        py::arg("polygon"), py::arg("linestring"));
    m.def("intersects_linestring_polygon", &intersects_ls_poly,   py::arg("linestring"), py::arg("polygon"));
    m.def("intersects_polygon_linestring", &intersects_poly_ls,   py::arg("polygon"), py::arg("linestring"));
    m.def("intersects_polygon_polygon",    &intersects_poly_poly, py::arg("polygon1"), py::arg("polygon2"));
    m.def("project_linestring_point",   &project_ls_pt,       py::arg("linestring"), py::arg("point"));
    m.def("interpolate_linestring",     &interpolate_ls,      py::arg("linestring"), py::arg("distance"));
    m.def("intersection_polygon_polygon",      &intersection_poly_poly,      py::arg("polygon1"), py::arg("polygon2"));
    m.def("intersection_area_polygon_polygon", &intersection_area_poly_poly, py::arg("polygon1"), py::arg("polygon2"));

    // ---- shapely.ops ---------
    m.def("nearest_points",      &nearest_poly_ls,   py::arg("polygon"), py::arg("linestring"));
    m.def("nearest_points_ls_pt",&nearest_ls_pt,     py::arg("linestring"), py::arg("point"));
}
