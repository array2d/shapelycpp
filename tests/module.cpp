// pybind11 test module for shapelycpp — demonstrates pycpp wrapper usage.
//
// Includes all shapely geometry types, cross-type predicates, distance,
// centroid, nearest_points, serialization, etc.
//
// Usage (consuming project):
//   #include "shapelycpp/pycpp/geometry_py.h"
//   #include "shapelycpp/pycpp/ops_py.h"
//   PYBIND11_MODULE(my_module, m) { ... bind classes + use BIND_PREDS macros ... }

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "../pycpp/geometry_py.h"
#include "../pycpp/ops_py.h"

#include <cstring>
#include <tuple>

namespace py = pybind11;
using namespace shapely::geometry;
using namespace shapely_py;

// Re-export native_to_array for module-local use (templated — supports float/double)
namespace {
template <typename T>
py::array_t<T> _native_to_array(const T* data, size_t rows, size_t cols) {
    return shapely_py::native_to_array(data, rows, cols);
}
}

PYBIND11_MODULE(shapelycpp, m) {
    m.doc() = "shapelycpp test module (powered by pycpp wrappers)";

    // -- Factories (from pycpp) --
    m.def("point",          &point,          py::arg("x"), py::arg("y"));
    m.def("point_f32",      &point_f32,      py::arg("x"), py::arg("y"));
    m.def("linestring",     &linestring,     py::arg("coords"));
    m.def("linestring_f32", &linestring_f32, py::arg("coords"));
    m.def("polygon",        &polygon,        py::arg("coords"));
    m.def("polygon_f32",    &polygon_f32,    py::arg("coords"));
    m.def("linearring",     &linearring,     py::arg("coords"));

    // ======================================================================
    // Point<double> — full API
    // ======================================================================
    py::class_<Point<double>>(m, "Point")
        .def(py::init<double, double>(), py::arg("x"), py::arg("y"))
        .def_readonly("x", &Point<double>::x)
        .def_readonly("y", &Point<double>::y)
        .def("coords", &Point<double>::coords)
        .def("xy",     &Point<double>::xy)
        .def("buffer", &Point<double>::buffer)
        .def("distance", [](const Point<double>& self, const Point<double>& o) {
            return self.distance(o);
        })
        BIND_ACCESSORS(Point<double>);

    // ======================================================================
    // Point<float> — minimal API
    // ======================================================================
    py::class_<Point<float>>(m, "PointF32")
        .def(py::init<float, float>(), py::arg("x"), py::arg("y"))
        .def_readonly("x", &Point<float>::x)
        .def_readonly("y", &Point<float>::y)
        .def("buffer",   &Point<float>::buffer)
        .def("is_valid", &Point<float>::is_valid)
        .def("distance", [](const Point<float>& self, const Point<float>& o) {
            return self.distance(o);
        });

    // ======================================================================
    // LineString<double> — full API
    // ======================================================================
    py::class_<LineString<double>>(m, "LineString")
        .def(py::init([](const py::array_t<double>& arr) {
            auto buf = arr.request();
            return new LineString<double>(static_cast<const double*>(buf.ptr),
                                           buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("coords",    &LineString<double>::coords)
        .def("xy",        &LineString<double>::xy)
        .def("is_closed", &LineString<double>::is_closed)
        .def("is_ring",   &LineString<double>::is_ring)
        .def("buffer",    &LineString<double>::buffer)
        .def("distance", [](const LineString<double>& self, const LineString<double>& o) {
            return self.distance(o);
        })
        .def("intersects", [](const LineString<double>& self, const LineString<double>& o) {
            return self.intersects(o);
        })
        BIND_ACCESSORS(LineString<double>);

    // ======================================================================
    // LineString<float> — minimal API
    // ======================================================================
    py::class_<LineString<float>>(m, "LineStringF32")
        .def(py::init([](const py::array_t<float>& arr) {
            auto buf = arr.request();
            return new LineString<float>(static_cast<const float*>(buf.ptr),
                                          buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("length",  &LineString<float>::length)
        .def("buffer",  &LineString<float>::buffer)
        .def("distance", [](const LineString<float>& self, const LineString<float>& o) {
            return self.distance(o);
        });

    // ======================================================================
    // Polygon<double> — full API
    // ======================================================================
    py::class_<Polygon<double>>(m, "Polygon")
        .def(py::init([](const py::array_t<double>& arr) {
            auto buf = arr.request();
            return new Polygon<double>(static_cast<const double*>(buf.ptr),
                                        buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("coords", &Polygon<double>::coords)
        .def("buffer", &Polygon<double>::buffer)
        .def("distance", [](const Polygon<double>& self, const Polygon<double>& o) {
            return self.distance(o);
        })
        .def("intersects", [](const Polygon<double>& self, const Polygon<double>& o) {
            return self.intersects(o);
        })
        .def_property_readonly("coords_arr", [](const Polygon<double>& poly) {
            return _native_to_array(poly.data(), poly.rows(), poly.cols());
        })
        BIND_ACCESSORS(Polygon<double>);

    // ======================================================================
    // Polygon<float> — minimal API
    // ======================================================================
    py::class_<Polygon<float>>(m, "PolygonF32")
        .def(py::init([](const py::array_t<float>& arr) {
            auto buf = arr.request();
            return new Polygon<float>(static_cast<const float*>(buf.ptr),
                                       buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("area",     &Polygon<float>::area)
        .def("is_valid", &Polygon<float>::is_valid)
        .def("is_empty", &Polygon<float>::is_empty)
        .def("buffer",   &Polygon<float>::buffer)
        .def("distance", [](const Polygon<float>& self, const Polygon<float>& o) {
            return self.distance(o);
        })
        .def_property_readonly("coords_arr", [](const Polygon<float>& poly) {
            return _native_to_array(poly.data(), poly.rows(), poly.cols());
        });

    // ======================================================================
    // LinearRing<double>
    // ======================================================================
    py::class_<LinearRing<double>>(m, "LinearRing")
        .def(py::init([](const py::array_t<double>& arr) {
            auto buf = arr.request();
            return new LinearRing<double>(static_cast<const double*>(buf.ptr),
                                           buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("coords",    &LinearRing<double>::coords)
        .def("xy",        &LinearRing<double>::xy)
        .def("is_closed", &LinearRing<double>::is_closed)
        .def("is_ring",   &LinearRing<double>::is_ring)
        .def("is_ccw",    &LinearRing<double>::is_ccw)
        BIND_ACCESSORS(LinearRing<double>);

    // ======================================================================
    // Cross-type predicates (via pycpp macros)
    // ======================================================================
    BIND_PREDS(m, pt, pt);
    BIND_PREDS(m, pt, ls);
    BIND_PREDS(m, pt, poly);
    BIND_PREDS(m, ls, pt);
    BIND_PREDS(m, ls, ls);
    BIND_PREDS(m, ls, poly);
    BIND_PREDS(m, poly, pt);
    BIND_PREDS(m, poly, ls);
    BIND_PREDS(m, poly, poly);

    // ======================================================================
    // Cross-type distance (from pycpp)
    // ======================================================================
    m.def("distance_point_linestring",   &distance_pt_ls);
    m.def("distance_point_polygon",      &distance_pt_poly);
    m.def("distance_linestring_polygon", &distance_ls_poly);
    m.def("distance_polygon_linestring", &distance_poly_ls);

    // ======================================================================
    // Cross-type intersects (from pycpp)
    // ======================================================================
    m.def("intersects_linestring_polygon", &intersects_ls_poly);
    m.def("intersects_polygon_linestring", &intersects_poly_ls);
    m.def("intersects_polygon_polygon",    &intersects_poly_poly);

    // ======================================================================
    // Project / interpolate (from pycpp)
    // ======================================================================
    m.def("project_linestring_point", &project_ls_pt);
    m.def("interpolate_linestring",   &interpolate_ls);

    // ======================================================================
    // Intersection area (from pycpp)
    // ======================================================================
    m.def("intersection_area_polygon_polygon", &intersection_area_poly_poly);

    // ======================================================================
    // Centroid (from pycpp)
    // ======================================================================
    m.def("centroid_point",      &centroid_point);
    m.def("centroid_linestring", &centroid_linestring);
    m.def("centroid_polygon",    &centroid_polygon);

    // ======================================================================
    // Polygon exterior (from pycpp)
    // ======================================================================
    m.def("polygon_exterior", &polygon_exterior);

    // ======================================================================
    // shapely.ops (from pycpp)
    // ======================================================================
    m.def("nearest_points",       &nearest_points_poly_ls);
    m.def("nearest_points_ls_pt", &nearest_points_ls_pt);
}
