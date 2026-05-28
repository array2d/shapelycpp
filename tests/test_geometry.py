"""
Precision alignment tests for shapely.geometry.* (Point, LineString, Polygon).

Each test compares the C++ implementation against Python shapely using
identical inputs and verifies results match within configured tolerance.

Same-type tests (Point↔Point, LineString↔LineString, Polygon↔Polygon) use
the parametrized ``make`` fixture to test both float64 and float32.
Cross-type tests use the ``cpp`` fixture (float64 only).
"""

import numpy as np
import pytest
from shapely.geometry import Point as PyPoint
from shapely.geometry import LineString as PyLineString
from shapely.geometry import Polygon as PyPolygon

from .utils import (compare, assert_match, tolerance_for,
                    make_square_coords, make_triangle_coords,
                    make_linestring_coords)


# ======================================================================
# Point API tests  (parametrized: float64 / float32)
# ======================================================================

class TestPointConstructor:
    """C++ Point(x,y) vs Python Point(x,y) — coordinate access."""

    def test_coordinates(self, make):
        atol = 1e-6 if make['dtype'] == np.float32 else 1e-14
        for x, y in [(0.0, 0.0), (1.5, -3.2), (-100.0, 200.0), (1e-10, -1e10)]:
            cpp_pt = make['point'](x, y)
            py_pt = PyPoint(x, y)
            assert abs(cpp_pt.x - py_pt.x) < atol, f"x mismatch: {cpp_pt.x} vs {py_pt.x}"
            assert abs(cpp_pt.y - py_pt.y) < atol, f"y mismatch: {cpp_pt.y} vs {py_pt.y}"


class TestPointDistancePoint:
    """C++ Point.distance(Point) vs Python Point.distance(Point)."""

    @pytest.mark.parametrize("x1,y1,x2,y2,expected", [
        (0.0, 0.0, 3.0, 4.0, 5.0),
        (0.0, 0.0, 1.0, 0.0, 1.0),
        (0.0, 0.0, 0.0, 1.0, 1.0),
        (1.0, 2.0, 4.0, 6.0, 5.0),
        (-1.0, -1.0, 2.0, 3.0, 5.0),
        (1e10, 0.0, 0.0, 0.0, 1e10),
        (0.0, 0.0, 0.0, 0.0, 0.0),
    ])
    def test_known_values(self, make, x1, y1, x2, y2, expected):
        atol = 1e-6 if make['dtype'] == np.float32 else 1e-10
        p1 = make['point'](x1, y1)
        p2 = make['point'](x2, y2)
        py_p1 = PyPoint(x1, y1)
        py_p2 = PyPoint(x2, y2)

        cpp_d = p1.distance(p2)
        py_d = py_p1.distance(py_p2)

        assert abs(cpp_d - py_d) < atol, f"distance: {cpp_d} vs {py_d}"
        assert abs(cpp_d - expected) < 1e-5, f"distance {cpp_d} != expected {expected}"

    def test_random_pairs(self, make, rtol, atol):
        rng = np.random.RandomState(42)
        tol = 1e-5 if make['dtype'] == np.float32 else 1e-10
        for _ in range(20):
            x1, y1 = rng.uniform(-100, 100), rng.uniform(-100, 100)
            x2, y2 = rng.uniform(-100, 100), rng.uniform(-100, 100)
            p1 = make['point'](x1, y1)
            p2 = make['point'](x2, y2)
            py_p1 = PyPoint(x1, y1)
            py_p2 = PyPoint(x2, y2)
            assert abs(p1.distance(p2) - py_p1.distance(py_p2)) < tol


class TestPointBuffer:
    """C++ Point.buffer(distance) vs Python Point.buffer(distance)."""

    @pytest.mark.parametrize("dist", [1.0, 5.0, 10.0, 0.1, 100.0])
    def test_buffer_area(self, make, dist):
        """Buffer of a point is a circle-like polygon; compare area."""
        p = make['point'](0.0, 0.0)
        py_p = PyPoint(0.0, 0.0)

        cpp_buf = p.buffer(dist)
        py_buf = py_p.buffer(dist)

        cpp_area = cpp_buf.area()
        py_area = py_buf.area
        assert abs(cpp_area - py_area) / max(py_area, 1e-10) < 0.01, \
            f"buffer area: cpp={cpp_area:.6f} py={py_area:.6f}"

    def test_buffer_zero(self, make):
        """Buffer(0) should produce empty polygon."""
        p = make['point'](0.0, 0.0)
        buf = p.buffer(0.0)
        assert buf.is_empty(), "buffer(0) should be empty"


# ======================================================================
# Point ↔ LineString / Point ↔ Polygon tests  (cross-type, float64 only)
# ======================================================================

class TestPointDistanceLineString:
    """C++ Point.distance(LineString) vs Python Point.distance(LineString)."""

    @pytest.mark.parametrize("px,py,coords,expected", [
        (0.0, 0.0, [(3.0, 0.0), (3.0, 4.0)], 3.0),
        (0.0, 0.0, [(1.0, 0.0), (1.0, 1.0)], 1.0),
        (0.0, 4.0, [(0.0, 0.0), (0.0, 10.0)], 0.0),
        (10.0, 0.0, [(-5.0, 0.0), (5.0, 0.0)], 5.0),
        (2.0, 2.0, [(0.0, 0.0), (4.0, 4.0)], 0.0),
    ])
    def test_known_values(self, cpp, px, py, coords, expected):
        pt = cpp.point(px, py)
        ls = cpp.linestring(coords)
        py_pt = PyPoint(px, py)
        py_ls = PyLineString(coords)

        cpp_d = cpp.distance_point_linestring(pt, ls)
        py_d = py_pt.distance(py_ls)
        assert abs(cpp_d - py_d) < 1e-8, f"dist: cpp={cpp_d} py={py_d}"

    def test_random(self, cpp):
        rng = np.random.RandomState(99)
        for _ in range(10):
            coords = [(rng.uniform(-10, 10), rng.uniform(-10, 10)) for _ in range(4)]
            px, py = rng.uniform(-10, 10), rng.uniform(-10, 10)
            pt = cpp.point(px, py)
            ls = cpp.linestring(coords)
            py_pt = PyPoint(px, py)
            py_ls = PyLineString(coords)
            assert abs(cpp.distance_point_linestring(pt, ls) - py_pt.distance(py_ls)) < 1e-8


class TestPointDistancePolygon:
    """C++ Point.distance(Polygon) vs Python Point.distance(Polygon)."""

    def test_outside(self, cpp):
        sq = make_square_coords()
        pt = cpp.point(10.0, 0.0)
        poly = cpp.polygon(sq)
        py_pt = PyPoint(10.0, 0.0)
        py_poly = PyPolygon(sq)
        cpp_d = cpp.distance_point_polygon(pt, poly)
        py_d = py_pt.distance(py_poly)
        assert abs(cpp_d - py_d) < 1e-8

    def test_inside(self, cpp):
        sq = make_square_coords()
        pt = cpp.point(0.0, 0.0)
        poly = cpp.polygon(sq)
        py_pt = PyPoint(0.0, 0.0)
        py_poly = PyPolygon(sq)
        cpp_d = cpp.distance_point_polygon(pt, poly)
        py_d = py_pt.distance(py_poly)
        assert abs(cpp_d - py_d) < 1e-8


# ======================================================================
# LineString API tests  (parametrized: float64 / float32)
# ======================================================================

class TestLineStringConstructor:
    """C++ LineString(coords) construction and length."""

    @pytest.mark.parametrize("coords,expected_len", [
        ([(0.0, 0.0), (1.0, 0.0)], 1.0),
        ([(0.0, 0.0), (3.0, 4.0)], 5.0),
        ([(0.0, 0.0), (1.0, 1.0), (2.0, 2.0)], 2.0 * np.sqrt(2)),
        ([(0.0, 0.0), (10.0, 0.0), (10.0, 10.0)], 20.0),
    ])
    def test_length(self, make, coords, expected_len):
        atol = 1e-6 if make['dtype'] == np.float32 else 1e-10
        ls = make['linestring'](coords)
        py_ls = PyLineString(coords)
        assert abs(ls.length() - py_ls.length) < atol
        assert abs(ls.length() - expected_len) < 1e-5


class TestLineStringDistanceLineString:
    """C++ LineString.distance(LineString) vs Python."""

    def test_parallel(self, make):
        l1 = [(0.0, 0.0), (0.0, 10.0)]
        l2 = [(3.0, 0.0), (3.0, 10.0)]
        ls1 = make['linestring'](l1)
        ls2 = make['linestring'](l2)
        py_ls1 = PyLineString(l1)
        py_ls2 = PyLineString(l2)
        atol = 1e-6 if make['dtype'] == np.float32 else 1e-8
        assert abs(ls1.distance(ls2) - py_ls1.distance(py_ls2)) < atol

    def test_intersecting(self, make):
        l1 = [(0.0, 0.0), (10.0, 10.0)]
        l2 = [(0.0, 10.0), (10.0, 0.0)]
        ls1 = make['linestring'](l1)
        ls2 = make['linestring'](l2)
        py_ls1 = PyLineString(l1)
        py_ls2 = PyLineString(l2)
        atol = 1e-6 if make['dtype'] == np.float32 else 1e-8
        assert abs(ls1.distance(ls2) - py_ls1.distance(py_ls2)) < atol

    def test_random(self, make):
        rng = np.random.RandomState(1)
        atol = 1e-5 if make['dtype'] == np.float32 else 1e-8
        for _ in range(10):
            c1 = [(rng.uniform(-10, 10), rng.uniform(-10, 10)) for _ in range(3)]
            c2 = [(rng.uniform(-10, 10), rng.uniform(-10, 10)) for _ in range(3)]
            ls1 = make['linestring'](c1)
            ls2 = make['linestring'](c2)
            py_ls1 = PyLineString(c1)
            py_ls2 = PyLineString(c2)
            assert abs(ls1.distance(ls2) - py_ls1.distance(py_ls2)) < atol


class TestLineStringBuffer:
    """C++ LineString.buffer(distance) vs Python."""

    def test_buffer_area(self, make):
        line = [(0.0, 0.0), (10.0, 0.0)]
        ls = make['linestring'](line)
        py_ls = PyLineString(line)
        buf = ls.buffer(1.0)
        py_buf = py_ls.buffer(1.0)
        assert abs(buf.area() - py_buf.area) / max(py_buf.area, 1e-10) < 0.01

    def test_buffer_zero(self, make):
        line = [(0.0, 0.0), (10.0, 0.0)]
        ls = make['linestring'](line)
        buf = ls.buffer(0.0)
        assert buf.is_empty() == True


# ======================================================================
# LineString ↔ Polygon cross-type tests (float64 only)
# ======================================================================

class TestLineStringDistancePolygon:
    """C++ LineString.distance(Polygon) vs Python."""

    def test_outside(self, cpp):
        sq = make_square_coords()
        line = [(10.0, 0.0), (10.0, 5.0)]
        ls = cpp.linestring(line)
        poly = cpp.polygon(sq)
        py_ls = PyLineString(line)
        py_poly = PyPolygon(sq)
        assert abs(cpp.distance_linestring_polygon(ls, poly) - py_ls.distance(py_poly)) < 1e-8

    def test_crossing(self, cpp):
        sq = make_square_coords()
        line = [(-10.0, 0.0), (10.0, 0.0)]
        ls = cpp.linestring(line)
        poly = cpp.polygon(sq)
        py_ls = PyLineString(line)
        py_poly = PyPolygon(sq)
        assert abs(cpp.distance_linestring_polygon(ls, poly) - py_ls.distance(py_poly)) < 1e-8


class TestLineStringIntersects:
    """C++ LineString.intersects(Polygon) vs Python."""

    def test_crosses(self, cpp):
        sq = make_square_coords()
        line = [(-10.0, 0.0), (10.0, 0.0)]
        ls = cpp.linestring(line)
        poly = cpp.polygon(sq)
        py_ls = PyLineString(line)
        py_poly = PyPolygon(sq)
        assert cpp.intersects_linestring_polygon(ls, poly) == py_ls.intersects(py_poly)

    def test_outside(self, cpp):
        sq = make_square_coords()
        line = [(10.0, 10.0), (15.0, 15.0)]
        ls = cpp.linestring(line)
        poly = cpp.polygon(sq)
        py_ls = PyLineString(line)
        py_poly = PyPolygon(sq)
        assert cpp.intersects_linestring_polygon(ls, poly) == py_ls.intersects(py_poly)

    def test_inside(self, cpp):
        sq = make_square_coords()
        line = [(-2.0, 0.0), (2.0, 0.0)]
        ls = cpp.linestring(line)
        poly = cpp.polygon(sq)
        py_ls = PyLineString(line)
        py_poly = PyPolygon(sq)
        assert cpp.intersects_linestring_polygon(ls, poly) == py_ls.intersects(py_poly)


class TestLineStringProject:
    """C++ LineString.project(Point) vs Python."""

    @pytest.mark.parametrize("line,px,py", [
        ([(0.0, 0.0), (10.0, 0.0)], 5.0, 0.0),
        ([(0.0, 0.0), (0.0, 10.0)], 0.0, 5.0),
        ([(0.0, 0.0), (10.0, 0.0)], 5.0, 3.0),
        ([(0.0, 0.0), (10.0, 0.0), (10.0, 10.0)], 10.0, 5.0),
    ])
    def test_values(self, cpp, line, px, py):
        ls = cpp.linestring(line)
        pt = cpp.point(px, py)
        py_ls = PyLineString(line)
        py_pt = PyPoint(px, py)
        cpp_val = cpp.project_linestring_point(ls, pt)
        py_val = py_ls.project(py_pt)
        assert abs(cpp_val - py_val) < 1e-8, f"project: cpp={cpp_val} py={py_val}"


class TestLineStringInterpolate:
    """C++ LineString.interpolate(distance) vs Python."""

    @pytest.mark.parametrize("line,dist,expected", [
        ([(0.0, 0.0), (10.0, 0.0)], 5.0, (5.0, 0.0)),
        ([(0.0, 0.0), (0.0, 10.0)], 5.0, (0.0, 5.0)),
        ([(0.0, 0.0), (10.0, 0.0)], 0.0, (0.0, 0.0)),
        ([(0.0, 0.0), (10.0, 0.0)], 10.0, (10.0, 0.0)),
    ])
    def test_values(self, cpp, line, dist, expected):
        ls = cpp.linestring(line)
        py_ls = PyLineString(line)
        x, y = cpp.interpolate_linestring(ls, dist)
        py_pt = py_ls.interpolate(dist)
        assert abs(x - py_pt.x) < 1e-8, f"x: {x} vs {py_pt.x}"
        assert abs(y - py_pt.y) < 1e-8, f"y: {y} vs {py_pt.y}"
        assert abs(x - expected[0]) < 1e-6
        assert abs(y - expected[1]) < 1e-6


# ======================================================================
# Polygon API tests  (parametrized: float64 / float32)
# ======================================================================

class TestPolygonConstructor:
    """C++ Polygon(coords) construction and properties."""

    def test_area_square(self, make):
        atol = 1e-6 if make['dtype'] == np.float32 else 1e-8
        sq = make_square_coords()
        poly = make['polygon'](sq)
        py_poly = PyPolygon(sq)
        assert abs(poly.area() - py_poly.area) < atol
        assert abs(poly.area() - 100.0) < 1e-5

    def test_area_triangle(self, make):
        atol = 1e-6 if make['dtype'] == np.float32 else 1e-8
        tri = make_triangle_coords()
        poly = make['polygon'](tri)
        py_poly = PyPolygon(tri)
        assert abs(poly.area() - py_poly.area) < atol
        assert abs(poly.area() - 50.0) < 1e-5

    def test_is_valid(self, make):
        sq = make_square_coords()
        poly = make['polygon'](sq)
        py_poly = PyPolygon(sq)
        assert poly.is_valid() == py_poly.is_valid

    def test_is_empty(self, make):
        sq = make_square_coords()
        poly = make['polygon'](sq)
        assert not poly.is_empty()

    def test_exterior_coords(self, make):
        atol = 1e-5 if make['dtype'] == np.float32 else 1e-10
        sq = make_square_coords()
        poly = make['polygon'](sq)
        coords = poly.exterior_coords()
        py_coords = np.array(PyPolygon(sq).exterior.coords)
        assert np.allclose(np.asarray(coords), py_coords, atol=atol)


class TestPolygonDistancePolygon:
    """C++ Polygon.distance(Polygon) vs Python."""

    def test_disjoint(self, make):
        atol = 1e-6 if make['dtype'] == np.float32 else 1e-8
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(20, 0, 5)
        p1 = make['polygon'](sq1)
        p2 = make['polygon'](sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        cpp_d = p1.distance(p2)
        py_d = py_p1.distance(py_p2)
        assert abs(cpp_d - py_d) < atol, f"dist: cpp={cpp_d} py={py_d}"
        assert abs(cpp_d - 10.0) < 1e-5

    def test_overlapping(self, make):
        atol = 1e-6 if make['dtype'] == np.float32 else 1e-8
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(3, 0, 5)
        p1 = make['polygon'](sq1)
        p2 = make['polygon'](sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        assert abs(p1.distance(p2) - py_p1.distance(py_p2)) < atol

    def test_touching(self, make):
        atol = 1e-6 if make['dtype'] == np.float32 else 1e-8
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(10, 0, 5)
        p1 = make['polygon'](sq1)
        p2 = make['polygon'](sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        assert abs(p1.distance(p2) - py_p1.distance(py_p2)) < atol


class TestPolygonBuffer:
    """C++ Polygon.buffer(distance) vs Python."""

    def test_buffer_area(self, make):
        sq = make_square_coords(0, 0, 5)
        poly = make['polygon'](sq)
        py_poly = PyPolygon(sq)
        buf = poly.buffer(2.0)
        py_buf = py_poly.buffer(2.0)
        assert abs(buf.area() - py_buf.area) / max(py_buf.area, 1e-10) < 0.01

    def test_buffer_zero(self, make):
        sq = make_square_coords(0, 0, 5)
        poly = make['polygon'](sq)
        py_poly = PyPolygon(sq)
        buf = poly.buffer(0.0)
        assert abs(buf.area() - py_poly.buffer(0.0).area) / max(py_poly.area, 1e-10) < 0.01


# ======================================================================
# Polygon cross-type tests (float64 only)
# ======================================================================

class TestPolygonDistanceLineString:
    """C++ Polygon.distance(LineString) vs Python."""

    def test_outside(self, cpp):
        sq = make_square_coords()
        line = [(10.0, 0.0), (10.0, 5.0)]
        poly = cpp.polygon(sq)
        ls = cpp.linestring(line)
        py_poly = PyPolygon(sq)
        py_ls = PyLineString(line)
        assert abs(cpp.distance_polygon_linestring(poly, ls) - py_poly.distance(py_ls)) < 1e-8


class TestPolygonIntersects:
    """C++ Polygon.intersects(Polygon) vs Python."""

    def test_overlapping(self, cpp):
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(3, 0, 5)
        p1 = cpp.polygon(sq1)
        p2 = cpp.polygon(sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        assert cpp.intersects_polygon_polygon(p1, p2) == py_p1.intersects(py_p2)

    def test_disjoint(self, cpp):
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(20, 0, 5)
        p1 = cpp.polygon(sq1)
        p2 = cpp.polygon(sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        assert cpp.intersects_polygon_polygon(p1, p2) == py_p1.intersects(py_p2)

    def test_contains(self, cpp):
        sq1 = make_square_coords(0, 0, 10)
        sq2 = make_square_coords(0, 0, 2)
        p1 = cpp.polygon(sq1)
        p2 = cpp.polygon(sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        assert cpp.intersects_polygon_polygon(p1, p2) == py_p1.intersects(py_p2)

    def test_with_linestring(self, cpp):
        sq = make_square_coords()
        line = [(-10.0, 0.0), (10.0, 0.0)]
        poly = cpp.polygon(sq)
        ls = cpp.linestring(line)
        py_poly = PyPolygon(sq)
        py_ls = PyLineString(line)
        assert cpp.intersects_polygon_linestring(poly, ls) == py_poly.intersects(py_ls)

        line2 = [(10.0, 10.0), (15.0, 15.0)]
        ls2 = cpp.linestring(line2)
        py_ls2 = PyLineString(line2)
        assert cpp.intersects_polygon_linestring(poly, ls2) == py_poly.intersects(py_ls2)


class TestPolygonIntersection:
    """C++ Polygon.intersection(Polygon) vs Python."""

    def test_overlap(self, cpp):
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(3, 0, 5)
        p1 = cpp.polygon(sq1)
        p2 = cpp.polygon(sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        cpp_inter_area = cpp.intersection_area_polygon_polygon(p1, p2)
        py_inter_area = py_p1.intersection(py_p2).area
        assert abs(cpp_inter_area - py_inter_area) / max(py_inter_area, 1e-10) < 0.01

    def test_no_overlap(self, cpp):
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(20, 0, 5)
        p1 = cpp.polygon(sq1)
        p2 = cpp.polygon(sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        cpp_inter_area = cpp.intersection_area_polygon_polygon(p1, p2)
        py_inter_area = py_p1.intersection(py_p2).area
        assert abs(cpp_inter_area - py_inter_area) < 1e-8
