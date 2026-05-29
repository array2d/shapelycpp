"""
Precision alignment tests for shapely.ops.* (nearest_points).

Each test compares the C++ implementation against Python shapely.ops using
identical inputs and verifies results match within configured tolerance.
"""

import numpy as np
import pytest
from shapely.geometry import Point as PyPoint
from shapely.geometry import LineString as PyLineString
from shapely.geometry import Polygon as PyPolygon
import shapely.ops as py_ops

from .utils import make_square_coords, make_triangle_coords


# ======================================================================
# nearest_points tests
# ======================================================================

class TestNearestPointsPolygonLineString:
    """C++ ops::nearest_points(Polygon, LineString) vs Python ops.nearest_points."""

    def test_disjoint(self, cpp):
        """Line is far from polygon."""
        sq = make_square_coords(0, 0, 5)   # (-5,-5)-(5,5)
        line = [(10.0, 0.0), (15.0, 0.0)]
        poly = cpp.polygon(sq)
        ls = cpp.linestring(line)
        py_poly = PyPolygon(sq)
        py_ls = PyLineString(line)

        x1, y1, x2, y2 = cpp.nearest_points(poly, ls)
        py_result = py_ops.nearest_points(py_poly, py_ls)

        assert x1 == py_result[0].x, f"x1: {x1} vs {py_result[0].x}"
        assert y1 == py_result[0].y, f"y1: {y1} vs {py_result[0].y}"
        assert x2 == py_result[1].x, f"x2: {x2} vs {py_result[1].x}"
        assert y2 == py_result[1].y, f"y2: {y2} vs {py_result[1].y}"

    def test_near(self, cpp):
        """Line is close to but not touching polygon."""
        sq = make_square_coords(0, 0, 5)   # (-5,-5)-(5,5)
        line = [(5.1, 0.0), (5.1, 10.0)]
        poly = cpp.polygon(sq)
        ls = cpp.linestring(line)
        py_poly = PyPolygon(sq)
        py_ls = PyLineString(line)

        x1, y1, x2, y2 = cpp.nearest_points(poly, ls)
        py_result = py_ops.nearest_points(py_poly, py_ls)

        assert x1 == py_result[0].x
        assert y1 == py_result[0].y
        assert x2 == py_result[1].x
        assert y2 == py_result[1].y

    def test_crossing(self, cpp):
        """Line crosses through polygon."""
        sq = make_square_coords(0, 0, 5)
        line = [(-10.0, 0.0), (10.0, 0.0)]
        poly = cpp.polygon(sq)
        ls = cpp.linestring(line)
        py_poly = PyPolygon(sq)
        py_ls = PyLineString(line)

        x1, y1, x2, y2 = cpp.nearest_points(poly, ls)
        py_result = py_ops.nearest_points(py_poly, py_ls)

        assert x1 == py_result[0].x
        assert y1 == py_result[0].y
        assert x2 == py_result[1].x
        assert y2 == py_result[1].y

    def test_random(self, cpp):
        rng = np.random.RandomState(777)
        for _ in range(10):
            cx, cy = rng.uniform(-50, 50), rng.uniform(-50, 50)
            half = rng.uniform(1, 10)
            sq = [(cx - half, cy - half), (cx + half, cy - half),
                  (cx + half, cy + half), (cx - half, cy + half)]
            line = [(rng.uniform(-100, 100), rng.uniform(-100, 100)),
                    (rng.uniform(-100, 100), rng.uniform(-100, 100))]
            poly = cpp.polygon(sq)
            ls = cpp.linestring(line)
            py_poly = PyPolygon(sq)
            py_ls = PyLineString(line)

            x1, y1, x2, y2 = cpp.nearest_points(poly, ls)
            py_result = py_ops.nearest_points(py_poly, py_ls)

            assert x1 == py_result[0].x
            assert y1 == py_result[0].y
            assert x2 == py_result[1].x
            assert y2 == py_result[1].y


class TestNearestPointsLineStringPoint:
    """C++ ops::nearest_points(LineString, Point) vs Python ops.nearest_points."""

    def test_point_on_line(self, cpp):
        line = [(0.0, 0.0), (10.0, 0.0)]
        ls = cpp.linestring(line)
        pt = cpp.point(5.0, 3.0)
        py_ls = PyLineString(line)
        py_pt = PyPoint(5.0, 3.0)

        x1, y1, x2, y2 = cpp.nearest_points_ls_pt(ls, pt)
        py_result = py_ops.nearest_points(py_ls, py_pt)

        assert x1 == py_result[0].x
        assert y1 == py_result[0].y
        assert x2 == py_result[1].x
        assert y2 == py_result[1].y

    def test_point_endpoint(self, cpp):
        line = [(0.0, 0.0), (10.0, 0.0)]
        ls = cpp.linestring(line)
        pt = cpp.point(15.0, 0.0)
        py_ls = PyLineString(line)
        py_pt = PyPoint(15.0, 0.0)

        x1, y1, x2, y2 = cpp.nearest_points_ls_pt(ls, pt)
        py_result = py_ops.nearest_points(py_ls, py_pt)

        assert x1 == py_result[0].x
        assert y1 == py_result[0].y
        assert x2 == py_result[1].x
        assert y2 == py_result[1].y

    def test_random(self, cpp):
        rng = np.random.RandomState(42)
        for _ in range(10):
            line = [(rng.uniform(-10, 10), rng.uniform(-10, 10)) for _ in range(3)]
            px, py = rng.uniform(-15, 15), rng.uniform(-15, 15)
            ls = cpp.linestring(line)
            pt = cpp.point(px, py)
            py_ls = PyLineString(line)
            py_pt = PyPoint(px, py)

            x1, y1, x2, y2 = cpp.nearest_points_ls_pt(ls, pt)
            py_result = py_ops.nearest_points(py_ls, py_pt)

            assert x1 == py_result[0].x
            assert y1 == py_result[0].y
            assert x2 == py_result[1].x
            assert y2 == py_result[1].y
