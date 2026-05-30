// pybind11 test module for shapelycpp — demonstrates pycpp wrapper usage.
//
// All factories are bound under a single Python name per geometry type
// (e.g. "point" handles float64 AND float32 via pybind11 overload resolution).
// No _f32 suffixes anywhere — dtype is selected by the input array/numpy type.

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

// Re-export native_to_array for module-local use (templated)
namespace {
template <typename T>
py::array_t<T> _native_to_array(const T* data, size_t rows, size_t cols) {
    return shapely_py::native_to_array(data, rows, cols);
}
}

PYBIND11_MODULE(shapelycpp, m) {
    m.doc() = "shapelycpp test module (powered by pycpp wrappers)";

    // -- Factories — all under unified names, dtype selected via input type --
    // point: scalar (x,y) → f64; array [x,y] with f32 dtype → f32
    m.def("point", py::overload_cast<double, double>(&point), py::arg("x"), py::arg("y"));
    m.def("point", py::overload_cast<const py::array_t<double>&>(&point), py::arg("coords"));
    m.def("point", py::overload_cast<const py::array_t<float>&>(&point),  py::arg("coords"));

    // linestring / polygon / linearring: array dtype selects f64 or f32
    m.def("linestring", py::overload_cast<const py::array_t<double>&>(&linestring), py::arg("coords"));
    m.def("linestring", py::overload_cast<const py::array_t<float>&>(&linestring),  py::arg("coords"));
    m.def("polygon",    py::overload_cast<const py::array_t<double>&>(&polygon),    py::arg("coords"));
    m.def("polygon",    py::overload_cast<const py::array_t<float>&>(&polygon),     py::arg("coords"));
    m.def("linearring", py::overload_cast<const py::array_t<double>&>(&linearring), py::arg("coords"));

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

    // -- Multi* factories (multipoint overloads by array dtype; multilinestring/multipolygon by vector<array_t<T>>) --
    m.def("multipoint", py::overload_cast<const py::array_t<double>&>(&multipoint), py::arg("coords"));
    m.def("multipoint", py::overload_cast<const py::array_t<float>&>(&multipoint),  py::arg("coords"));
    m.def("multilinestring", py::overload_cast<const std::vector<py::array_t<double>>&>(&multilinestring), py::arg("lines"));
    m.def("multilinestring", py::overload_cast<const std::vector<py::array_t<float>>&>(&multilinestring),  py::arg("lines"));
    m.def("multipolygon", py::overload_cast<const std::vector<py::array_t<double>>&>(&multipolygon), py::arg("polygons"));
    m.def("multipolygon", py::overload_cast<const std::vector<py::array_t<float>>&>(&multipolygon),  py::arg("polygons"));

    // ======================================================================
    // MultiPoint<double> — full API
    // ======================================================================
    py::class_<MultiPoint<double>>(m, "MultiPoint")
        .def(py::init([](const py::array_t<double>& arr) {
            auto buf = arr.request();
            return new MultiPoint<double>(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
        }), py::arg("coords"))
        .def("num_geometries", &MultiPoint<double>::num_geometries)
        .def("geometry_n", &MultiPoint<double>::geometry_n)
        .def("distance", [](const MultiPoint<double>& self, const Point<double>& o) {
            return self.distance(o);
        })
        .def("distance", [](const MultiPoint<double>& self, const MultiPoint<double>& o) {
            return self.distance(o);
        })
        .def("intersects", [](const MultiPoint<double>& self, const Point<double>& o) {
            return self.intersects(o);
        })
        .def("intersects", [](const MultiPoint<double>& self, const MultiPoint<double>& o) {
            return self.intersects(o);
        })
        BIND_ACCESSORS(MultiPoint<double>);

    // ======================================================================
    // MultiLineString<double> — full API
    // ======================================================================
    py::class_<MultiLineString<double>>(m, "MultiLineString")
        .def(py::init([](const std::vector<py::array_t<double>>& arrays) {
            auto* mls = new MultiLineString<double>();
            for (auto& arr : arrays) {
                auto buf = arr.request();
                mls->add_line(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
            }
            return mls;
        }), py::arg("lines"))
        .def("num_geometries", &MultiLineString<double>::num_geometries)
        .def("geometry_n", &MultiLineString<double>::geometry_n)
        .def("add_line", [](MultiLineString<double>& self, const py::array_t<double>& arr) {
            auto buf = arr.request();
            self.add_line(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
        })
        .def("distance", [](const MultiLineString<double>& self, const Point<double>& o) {
            return self.distance(o);
        })
        .def("distance", [](const MultiLineString<double>& self, const LineString<double>& o) {
            return self.distance(o);
        })
        .def("distance", [](const MultiLineString<double>& self, const MultiLineString<double>& o) {
            return self.distance(o);
        })
        .def("intersects", [](const MultiLineString<double>& self, const Point<double>& o) {
            return self.intersects(o);
        })
        .def("intersects", [](const MultiLineString<double>& self, const LineString<double>& o) {
            return self.intersects(o);
        })
        .def("intersects", [](const MultiLineString<double>& self, const MultiLineString<double>& o) {
            return self.intersects(o);
        })
        BIND_ACCESSORS(MultiLineString<double>);

    // ======================================================================
    // MultiPolygon<double> — full API
    // ======================================================================
    py::class_<MultiPolygon<double>>(m, "MultiPolygon")
        .def(py::init([](const std::vector<py::array_t<double>>& arrays) {
            auto* mp = new MultiPolygon<double>();
            for (auto& arr : arrays) {
                auto buf = arr.request();
                mp->add_polygon(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
            }
            return mp;
        }), py::arg("polygons"))
        .def("num_geometries", &MultiPolygon<double>::num_geometries)
        .def("geometry_n", &MultiPolygon<double>::geometry_n)
        .def("add_polygon", [](MultiPolygon<double>& self, const py::array_t<double>& arr) {
            auto buf = arr.request();
            self.add_polygon(static_cast<const double*>(buf.ptr), buf.shape[0], buf.shape[1]);
        })
        .def("distance", [](const MultiPolygon<double>& self, const Point<double>& o) {
            return self.distance(o);
        })
        .def("distance", [](const MultiPolygon<double>& self, const LineString<double>& o) {
            return self.distance(o);
        })
        .def("distance", [](const MultiPolygon<double>& self, const Polygon<double>& o) {
            return self.distance(o);
        })
        .def("distance", [](const MultiPolygon<double>& self, const MultiPolygon<double>& o) {
            return self.distance(o);
        })
        .def("intersects", [](const MultiPolygon<double>& self, const Point<double>& o) {
            return self.intersects(o);
        })
        .def("intersects", [](const MultiPolygon<double>& self, const LineString<double>& o) {
            return self.intersects(o);
        })
        .def("intersects", [](const MultiPolygon<double>& self, const Polygon<double>& o) {
            return self.intersects(o);
        })
        .def("intersects", [](const MultiPolygon<double>& self, const MultiPolygon<double>& o) {
            return self.intersects(o);
        })
        BIND_ACCESSORS(MultiPolygon<double>);

    // ======================================================================
    // GeometryCollection — basic API
    // ======================================================================
    py::class_<GeometryCollection>(m, "GeometryCollection")
        .def(py::init<>())
        .def("num_geometries", &GeometryCollection::num_geometries)
        .def("is_empty", &GeometryCollection::is_empty)
        .def("area", &GeometryCollection::area)
        .def("length", &GeometryCollection::length)
        .def("wkt", &GeometryCollection::wkt)
        .def("buffer", &GeometryCollection::buffer)
        .def("normalize", &GeometryCollection::normalize);

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
