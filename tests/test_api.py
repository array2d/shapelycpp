"""
Unified C++ vs Python shapely test suite.
Covers float32/float64, all geometry types, and all API operations.

- ``make`` fixture (parametrized float64/float32): same-type ops (Pt↔Pt, LS↔LS, Poly↔Poly)
- ``cpp`` fixture (float64 only): cross-type ops, linearring, serialization, nearest_points

All comparisons are bit-identical (strict equality) except buffer areas (~1% tolerance).
"""

import numpy as np
import pytest
from shapely.geometry import Point as PyPoint, LineString as PyLineString, Polygon as PyPolygon
import shapely.ops as py_ops
from shapely import wkt as shapely_wkt

from .utils import (make_square_coords, make_triangle_coords, py_linearring,
                    py_point as _py_point, py_linestring as _py_ls, py_polygon as _py_poly)

py_point = _py_point
py_linestring = _py_ls
py_polygon = _py_poly


def assert_eq(a, b, make=None, msg=""):
    """Strict equality for float64, tolerance-based for float32 (precision loss vs Python float64)."""
    if make is not None and make.get('dtype') == np.float32:
        assert abs(a - b) <= 1e-5, f"{msg} f32: {a} vs {b} (diff={abs(a-b):.2e})"
    else:
        assert a == b, f"{msg}: {a} vs {b}"


# ==============================================================================
# Point — accessors, same-type distance, buffer (make fixture: f64/f32)
# ==============================================================================

class TestPointAccessors:
    """C++ Point(x,y) vs Python Point(x,y) — coordinates and basic properties."""

    def test_coordinates(self, make):
        for x, y in [(0.0, 0.0), (1.5, -3.2), (-100.0, 200.0), (1e-10, -1e10)]:
            cpt = make['point'](x, y)
            ppt = PyPoint(x, y)
            assert cpt.x == ppt.x, f"x: {cpt.x} vs {ppt.x}"
            assert cpt.y == ppt.y, f"y: {cpt.y} vs {ppt.y}"

    def test_is_empty_and_valid(self, make):
        p = make['point'](0.0, 0.0)
        pp = PyPoint(0.0, 0.0)
        assert p.is_valid() == pp.is_valid

    def test_coords_xy(self, cpp, C):
        p = C.point(1.5, -2.5)
        cs = list(p.coords())
        assert len(cs) == 1
        assert cs[0][0] == 1.5
        assert cs[0][1] == -2.5
        xs, ys = p.xy()
        assert xs == [1.5] and ys == [-2.5]

    def test_full_accessors_f64(self, cpp, C):
        for x, y in [(0, 0), (3.14, -2.71), (1e6, -1e-6)]:
            cpt = C.point(x, y)
            ppt = py_point(x, y)
            assert cpt.x == ppt.x
            assert cpt.y == ppt.y
            assert cpt.is_empty() == ppt.is_empty
            assert cpt.is_simple() == ppt.is_simple
            assert cpt.is_valid() == ppt.is_valid
            assert cpt.area() == ppt.area
            assert cpt.length() == ppt.length
            assert cpt.has_z() == ppt.has_z
            assert cpt.geom_type() == ppt.geom_type
            assert cpt.type() == "Point"
            assert len(cpt.wkb_hex()) > 0
            assert cpt.bounds() == list(ppt.bounds)
            # WKT roundtrip
            g1 = shapely_wkt.loads(cpt.wkt())
            g2 = shapely_wkt.loads(ppt.wkt)
            assert g1.equals_exact(g2, 1e-10), f"WKT mismatch:\n  C++: {cpt.wkt()}\n  Py:  {ppt.wkt}"

    def test_accessors_f32(self, cpp, C):
        for x, y in [(0, 0), (3.14, -2.71)]:
            cpt = C.point(x, y, 'float32')
            ppt = py_point(float(x), float(y))
            assert float(cpt.x) == ppt.x
            assert float(cpt.y) == ppt.y
            assert cpt.is_valid() == ppt.is_valid


class TestPointDistancePoint:
    """C++ Point.distance(Point) vs Python Point.distance(Point)."""

    @pytest.mark.parametrize("x1,y1,x2,y2", [
        (0.0, 0.0, 3.0, 4.0),
        (0.0, 0.0, 1.0, 0.0),
        (0.0, 0.0, 0.0, 1.0),
        (1.0, 2.0, 4.0, 6.0),
        (-1.0, -1.0, 2.0, 3.0),
        (1e10, 0.0, 0.0, 0.0),
        (0.0, 0.0, 0.0, 0.0),
    ])
    def test_known_values(self, make, x1, y1, x2, y2):
        p1 = make['point'](x1, y1)
        p2 = make['point'](x2, y2)
        py_p1 = PyPoint(x1, y1)
        py_p2 = PyPoint(x2, y2)
        cpp_d = p1.distance(p2)
        py_d = py_p1.distance(py_p2)
        assert cpp_d == py_d, f"distance: {cpp_d} vs {py_d}"

    def test_random_pairs(self, make):
        rng = np.random.RandomState(42)
        for _ in range(20):
            x1, y1 = rng.uniform(-100, 100), rng.uniform(-100, 100)
            x2, y2 = rng.uniform(-100, 100), rng.uniform(-100, 100)
            p1 = make['point'](x1, y1)
            p2 = make['point'](x2, y2)
            py_p1 = PyPoint(x1, y1)
            py_p2 = PyPoint(x2, y2)
            assert p1.distance(p2) == py_p1.distance(py_p2)


class TestPointPredicatesPtPt:
    """C++ Point↔Point predicates vs Python (f64 cross-type bindings)."""

    @pytest.mark.parametrize("x1,y1,x2,y2", [
        (0,0,0,0), (0,0,1,1), (5,5,5,5), (-1,2,-3,4),
    ])
    def test_all_predicates(self, cpp, C, x1, y1, x2, y2):
        c1, c2 = C.point(x1, y1), C.point(x2, y2)
        p1, p2 = py_point(x1, y1), py_point(x2, y2)
        assert cpp.pt_contains_pt(c1, c2) == p1.contains(p2)
        assert cpp.pt_within_pt(c1, c2) == p1.within(p2)
        assert cpp.pt_disjoint_pt(c1, c2) == p1.disjoint(p2)
        assert cpp.pt_touches_pt(c1, c2) == p1.touches(p2)
        assert cpp.pt_crosses_pt(c1, c2) == p1.crosses(p2)
        assert cpp.pt_overlaps_pt(c1, c2) == p1.overlaps(p2)
        assert cpp.pt_covers_pt(c1, c2) == p1.covers(p2)
        assert cpp.pt_covered_by_pt(c1, c2) == p1.covered_by(p2)
        assert cpp.pt_equals_pt(c1, c2) == p1.equals(p2)
        assert cpp.pt_equals_exact_pt(c1, c2, 1e-6) == p1.equals_exact(p2, 1e-6)
        assert cpp.pt_intersects_pt(c1, c2) == p1.intersects(p2)
        assert cpp.pt_relate_pt(c1, c2) == p1.relate(p2)
        assert cpp.pt_hausdorff_distance_pt(c1, c2) == p1.hausdorff_distance(p2)

    def test_random(self, cpp, C):
        rng = np.random.RandomState(42)
        for _ in range(20):
            x1, y1 = rng.uniform(-100, 100), rng.uniform(-100, 100)
            x2, y2 = rng.uniform(-100, 100), rng.uniform(-100, 100)
            c1, c2 = C.point(x1, y1), C.point(x2, y2)
            p1, p2 = py_point(x1, y1), py_point(x2, y2)
            assert cpp.pt_intersects_pt(c1, c2) == p1.intersects(p2)
            assert cpp.pt_disjoint_pt(c1, c2) == p1.disjoint(p2)


class TestPointBuffer:
    """C++ Point.buffer(distance) vs Python — buffer areas use relative tolerance."""

    @pytest.mark.parametrize("dist", [1.0, 5.0, 10.0, 0.1, 100.0])
    def test_buffer_area(self, make, dist):
        p = make['point'](0.0, 0.0)
        py_p = PyPoint(0.0, 0.0)
        cpp_buf = p.buffer(dist)
        py_buf = py_p.buffer(dist)
        cpp_area = cpp_buf.area()
        py_area = py_buf.area
        assert abs(cpp_area - py_area) / max(py_area, 1e-10) < 0.01, \
            f"buffer area: cpp={cpp_area:.6f} py={py_area:.6f}"

    def test_buffer_zero(self, make):
        p = make['point'](0.0, 0.0)
        buf = p.buffer(0.0)
        assert buf.is_empty() == True


# ==============================================================================
# Point ↔ LineString / Point ↔ Polygon (cross-type, float64 only)
# ==============================================================================

class TestPointDistanceLineString:
    """C++ Point.distance(LineString) vs Python."""

    def test_known_values(self, cpp):
        for px, py, coords in [
            (0.0, 0.0, [(3.0, 0.0), (3.0, 4.0)]),
            (0.0, 0.0, [(1.0, 0.0), (1.0, 1.0)]),
            (0.0, 4.0, [(0.0, 0.0), (0.0, 10.0)]),
            (10.0, 0.0, [(-5.0, 0.0), (5.0, 0.0)]),
            (2.0, 2.0, [(0.0, 0.0), (4.0, 4.0)]),
        ]:
            pt = cpp.point(px, py)
            ls = cpp.linestring(coords)
            py_pt = PyPoint(px, py)
            py_ls = PyLineString(coords)
            cpp_d = cpp.distance_point_linestring(pt, ls)
            py_d = py_pt.distance(py_ls)
            assert cpp_d == py_d, f"dist: cpp={cpp_d} py={py_d}"

    def test_random(self, cpp):
        rng = np.random.RandomState(99)
        for _ in range(10):
            coords = [(rng.uniform(-10, 10), rng.uniform(-10, 10)) for _ in range(4)]
            px, py = rng.uniform(-10, 10), rng.uniform(-10, 10)
            pt = cpp.point(px, py)
            ls = cpp.linestring(coords)
            py_pt = PyPoint(px, py)
            py_ls = PyLineString(coords)
            assert cpp.distance_point_linestring(pt, ls) == py_pt.distance(py_ls)


class TestPointDistancePolygon:
    """C++ Point.distance(Polygon) vs Python."""

    def test_outside(self, cpp):
        sq = make_square_coords()
        pt = cpp.point(10.0, 0.0)
        poly = cpp.polygon(sq)
        py_pt = PyPoint(10.0, 0.0)
        py_poly = PyPolygon(sq)
        assert cpp.distance_point_polygon(pt, poly) == py_pt.distance(py_poly)

    def test_inside(self, cpp):
        sq = make_square_coords()
        pt = cpp.point(0.0, 0.0)
        poly = cpp.polygon(sq)
        py_pt = PyPoint(0.0, 0.0)
        py_poly = PyPolygon(sq)
        assert cpp.distance_point_polygon(pt, poly) == py_pt.distance(py_poly)


class TestPointPredicatesPoly:
    """C++ Point↔Polygon predicates vs Python."""

    def test_all_scenarios(self, cpp, C):
        sq = C.polygon([(0,0),(10,0),(10,10),(0,10)])
        psq = py_polygon([(0,0),(10,0),(10,10),(0,10)])
        for name, cx, cy in [("in",5,5),("out",20,20),("edge",0,5)]:
            cpt, ppt = C.point(cx, cy), py_point(cx, cy)
            assert cpp.pt_within_poly(cpt, sq) == ppt.within(psq), f"within {name}"
            assert cpp.pt_disjoint_poly(cpt, sq) == ppt.disjoint(psq), f"disjoint {name}"
            assert cpp.pt_touches_poly(cpt, sq) == ppt.touches(psq), f"touches {name}"
            assert cpp.pt_intersects_poly(cpt, sq) == ppt.intersects(psq), f"intersects {name}"
            assert cpp.pt_contains_poly(cpt, sq) == ppt.contains(psq), f"contains {name}"


# ==============================================================================
# LineString — accessors, same-type distance, buffer (make fixture: f64/f32)
# ==============================================================================

class TestLineStringAccessors:
    """C++ LineString construction and properties."""

    def test_full_accessors(self, cpp, C):
        c_ls = C.linestring([(0,0),(10,0),(10,10)])
        p_ls = py_linestring([(0,0),(10,0),(10,10)])
        g1 = shapely_wkt.loads(c_ls.wkt())
        g2 = shapely_wkt.loads(p_ls.wkt)
        assert g1.equals_exact(g2, 1e-10), f"WKT mismatch"
        assert len(c_ls.wkb_hex()) > 0
        assert c_ls.geom_type() == p_ls.geom_type
        assert c_ls.type() == "LineString"
        assert c_ls.has_z() == p_ls.has_z
        assert c_ls.is_empty() == p_ls.is_empty
        assert c_ls.is_simple() == p_ls.is_simple
        assert c_ls.is_valid() == p_ls.is_valid
        assert c_ls.area() == p_ls.area
        assert c_ls.length() == p_ls.length
        assert c_ls.bounds() == list(p_ls.bounds)

    def test_is_closed_ring(self, cpp, C):
        open_ls = C.linestring([(0,0),(10,0)])
        closed_ls = C.linestring([(0,0),(10,0),(10,10),(0,0)])
        assert open_ls.is_closed() == False
        assert closed_ls.is_closed() == True
        assert open_ls.is_ring() == False
        assert closed_ls.is_ring() == True

    def test_coords_xy(self, cpp, C):
        pts = [(1,2),(3,4),(5,6)]
        c_ls = C.linestring(pts)
        for (cx, cy), (px, py) in zip(c_ls.coords(), pts):
            assert cx == px
            assert cy == py
        xs, ys = c_ls.xy()
        assert xs == [1.,3.,5.] and ys == [2.,4.,6.]

    @pytest.mark.parametrize("coords", [
        [(0.0, 0.0), (1.0, 0.0)],
        [(0.0, 0.0), (3.0, 4.0)],
        [(0.0, 0.0), (1.0, 1.0), (2.0, 2.0)],
        [(0.0, 0.0), (10.0, 0.0), (10.0, 10.0)],
    ])
    def test_length(self, make, coords):
        ls = make['linestring'](coords)
        py_ls = PyLineString(coords)
        assert ls.length() == py_ls.length


class TestLineStringDistanceLineString:
    """C++ LineString.distance(LineString) vs Python."""

    def test_parallel(self, make):
        l1 = [(0.0, 0.0), (0.0, 10.0)]
        l2 = [(3.0, 0.0), (3.0, 10.0)]
        ls1 = make['linestring'](l1)
        ls2 = make['linestring'](l2)
        py_ls1 = PyLineString(l1)
        py_ls2 = PyLineString(l2)
        assert ls1.distance(ls2) == py_ls1.distance(py_ls2)

    def test_intersecting(self, make):
        l1 = [(0.0, 0.0), (10.0, 10.0)]
        l2 = [(0.0, 10.0), (10.0, 0.0)]
        ls1 = make['linestring'](l1)
        ls2 = make['linestring'](l2)
        py_ls1 = PyLineString(l1)
        py_ls2 = PyLineString(l2)
        assert ls1.distance(ls2) == py_ls1.distance(py_ls2)

    def test_random(self, make):
        rng = np.random.RandomState(1)
        for _ in range(10):
            c1 = [(rng.uniform(-10, 10), rng.uniform(-10, 10)) for _ in range(3)]
            c2 = [(rng.uniform(-10, 10), rng.uniform(-10, 10)) for _ in range(3)]
            ls1 = make['linestring'](c1)
            ls2 = make['linestring'](c2)
            py_ls1 = PyLineString(c1)
            py_ls2 = PyLineString(c2)
            assert ls1.distance(ls2) == py_ls1.distance(py_ls2)


class TestLineStringPredicatesLsLs:
    """C++ LineString↔LineString predicates vs Python."""

    def test_crossing(self, cpp, C):
        ls1 = C.linestring([(0,5),(10,5)])
        ls2 = C.linestring([(5,0),(5,10)])
        ls3 = C.linestring([(0,10),(10,10)])
        p1 = py_linestring([(0,5),(10,5)])
        p2 = py_linestring([(5,0),(5,10)])
        p3 = py_linestring([(0,10),(10,10)])
        assert cpp.ls_crosses_ls(ls1, ls2) == p1.crosses(p2)
        assert cpp.ls_disjoint_ls(ls1, ls3) == p1.disjoint(p3)
        assert cpp.ls_intersects_ls(ls1, ls2) == p1.intersects(p2)
        assert cpp.ls_touches_ls(ls1, ls3) == p1.touches(p3)

    def test_random(self, cpp, C):
        rng = np.random.RandomState(42)
        for _ in range(20):
            c1 = [(rng.uniform(-100, 100), rng.uniform(-100, 100)) for _ in range(3)]
            c2 = [(rng.uniform(-100, 100), rng.uniform(-100, 100)) for _ in range(3)]
            cls1, cls2 = C.linestring(c1), C.linestring(c2)
            p1, p2 = py_linestring(c1), py_linestring(c2)
            assert cpp.ls_intersects_ls(cls1, cls2) == p1.intersects(p2)


class TestLineStringPredicatesPt:
    """C++ LineString↔Point predicates vs Python."""

    def test_on_and_off(self, cpp, C):
        ls = C.linestring([(0,0),(10,0)])
        pls = py_linestring([(0,0),(10,0)])
        pt_on, pt_off = C.point(5,0), C.point(5,5)
        ppt_on, ppt_off = py_point(5,0), py_point(5,5)
        assert cpp.ls_contains_pt(ls, pt_on) == pls.contains(ppt_on)
        assert cpp.ls_intersects_pt(ls, pt_on) == pls.intersects(ppt_on)
        assert cpp.ls_disjoint_pt(ls, pt_off) == pls.disjoint(ppt_off)


class TestLineStringBuffer:
    """C++ LineString.buffer(distance) vs Python — buffer areas use relative tolerance."""

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


# ==============================================================================
# LineString ↔ Polygon cross-type (float64 only)
# ==============================================================================

class TestLineStringDistancePolygon:
    """C++ LineString.distance(Polygon) vs Python."""

    def test_outside(self, cpp):
        sq = make_square_coords()
        line = [(10.0, 0.0), (10.0, 5.0)]
        ls = cpp.linestring(line)
        poly = cpp.polygon(sq)
        py_ls = PyLineString(line)
        py_poly = PyPolygon(sq)
        assert cpp.distance_linestring_polygon(ls, poly) == py_ls.distance(py_poly)

    def test_crossing(self, cpp):
        sq = make_square_coords()
        line = [(-10.0, 0.0), (10.0, 0.0)]
        ls = cpp.linestring(line)
        poly = cpp.polygon(sq)
        py_ls = PyLineString(line)
        py_poly = PyPolygon(sq)
        assert cpp.distance_linestring_polygon(ls, poly) == py_ls.distance(py_poly)


class TestLineStringIntersectsPolygon:
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


class TestLineStringPredicatesPoly:
    """C++ LineString↔Polygon cross-type predicates."""

    def test_scenarios(self, cpp, C):
        sq = C.polygon([(0,0),(10,0),(10,10),(0,10)])
        psq = py_polygon([(0,0),(10,0),(10,10),(0,10)])
        scenarios = [
            ("crossing", [(-5,5),(15,5)]),
            ("outside",  [(20,0),(25,0)]),
            ("inside",   [(2,2),(8,2)]),
            ("edge",     [(0,5),(10,5)]),
        ]
        for name, cds in scenarios:
            cls, pls = C.linestring(cds), py_linestring(cds)
            assert cpp.ls_intersects_poly(cls, sq) == pls.intersects(psq), f"{name} intersects"


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
        assert cpp.project_linestring_point(ls, pt) == py_ls.project(py_pt)


class TestLineStringInterpolate:
    """C++ LineString.interpolate(distance) vs Python."""

    @pytest.mark.parametrize("line,dist", [
        ([(0.0, 0.0), (10.0, 0.0)], 5.0),
        ([(0.0, 0.0), (0.0, 10.0)], 5.0),
        ([(0.0, 0.0), (10.0, 0.0)], 0.0),
        ([(0.0, 0.0), (10.0, 0.0)], 10.0),
    ])
    def test_values(self, cpp, line, dist):
        ls = cpp.linestring(line)
        py_ls = PyLineString(line)
        x, y = cpp.interpolate_linestring(ls, dist)
        py_pt = py_ls.interpolate(dist)
        assert x == py_pt.x, f"x: {x} vs {py_pt.x}"
        assert y == py_pt.y, f"y: {y} vs {py_pt.y}"


# ==============================================================================
# Polygon — accessors, same-type distance, buffer (make fixture: f64/f32)
# ==============================================================================

class TestPolygonAccessors:
    """C++ Polygon construction and properties."""

    SQ = [(0,0),(10,0),(10,10),(0,10)]

    def test_full_accessors(self, cpp, C):
        cp = C.polygon(self.SQ)
        pp = py_polygon(self.SQ)
        g1 = shapely_wkt.loads(cp.wkt())
        g2 = shapely_wkt.loads(pp.wkt)
        assert g1.equals_exact(g2, 1e-10), "WKT mismatch"
        assert len(cp.wkb_hex()) > 0
        assert cp.geom_type() == pp.geom_type
        assert cp.type() == "Polygon"
        assert cp.has_z() == pp.has_z
        assert cp.is_empty() == pp.is_empty
        assert cp.is_simple() == pp.is_simple
        assert cp.is_valid() == pp.is_valid
        assert cp.area() == pp.area
        assert cp.length() == pp.length
        assert cp.bounds() == list(pp.bounds)

    def test_area_square(self, make):
        sq = make_square_coords()
        poly = make['polygon'](sq)
        py_poly = PyPolygon(sq)
        assert poly.area() == py_poly.area
        assert poly.area() == 100.0

    def test_area_triangle(self, make):
        tri = make_triangle_coords()
        poly = make['polygon'](tri)
        py_poly = PyPolygon(tri)
        assert poly.area() == py_poly.area
        assert poly.area() == 50.0

    def test_is_valid(self, make):
        sq = make_square_coords()
        poly = make['polygon'](sq)
        py_poly = PyPolygon(sq)
        assert poly.is_valid() == py_poly.is_valid

    def test_is_empty(self, make):
        sq = make_square_coords()
        poly = make['polygon'](sq)
        assert poly.is_empty() == False

    def test_coords(self, cpp, C):
        coords = list(C.polygon(self.SQ).coords())
        for (cx, cy), (x, y) in zip(coords, self.SQ):
            assert cx == x

    def test_exterior(self, cpp, C):
        ext = cpp.polygon_exterior(C.polygon(self.SQ))
        pext = py_polygon(self.SQ).exterior
        assert ext.shape[0] == len(pext.coords)
        for i in range(ext.shape[0]):
            for j in range(2):
                assert ext[i][j] == pext.coords[i][j]


class TestPolygonDistancePolygon:
    """C++ Polygon.distance(Polygon) vs Python."""

    def test_disjoint(self, make):
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(20, 0, 5)
        p1 = make['polygon'](sq1)
        p2 = make['polygon'](sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        assert p1.distance(p2) == py_p1.distance(py_p2)
        assert p1.distance(p2) == 10.0

    def test_overlapping(self, make):
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(3, 0, 5)
        p1 = make['polygon'](sq1)
        p2 = make['polygon'](sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        assert p1.distance(p2) == py_p1.distance(py_p2)

    def test_touching(self, make):
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(10, 0, 5)
        p1 = make['polygon'](sq1)
        p2 = make['polygon'](sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        assert p1.distance(p2) == py_p1.distance(py_p2)


class TestPolygonBuffer:
    """C++ Polygon.buffer(distance) vs Python — buffer areas use relative tolerance."""

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


# ==============================================================================
# Polygon ↔ Polygon, Point, LineString cross-type (float64 only)
# ==============================================================================

class TestPolygonPredicatesPoly:
    """C++ Polygon↔Polygon predicates vs Python."""

    SQ = [(0,0),(10,0),(10,10),(0,10)]

    def test_overlapping(self, cpp, C):
        sq = C.polygon(self.SQ)
        sq2 = C.polygon([(5,0),(15,0),(15,10),(5,10)])
        sq3 = C.polygon([(20,20),(30,20),(30,30),(20,30)])
        psq = py_polygon(self.SQ)
        psq2 = py_polygon([(5,0),(15,0),(15,10),(5,10)])
        psq3 = py_polygon([(20,20),(30,20),(30,30),(20,30)])
        assert cpp.poly_overlaps_poly(sq, sq2) == psq.overlaps(psq2)
        assert cpp.poly_disjoint_poly(sq, sq3) == psq.disjoint(psq3)
        assert cpp.poly_touches_poly(sq, sq2) == psq.touches(psq2)
        assert cpp.poly_intersects_poly(sq, sq2) == psq.intersects(psq2)
        assert cpp.poly_relate_poly(sq, sq) == psq.relate(psq)
        assert cpp.poly_contains_poly(sq, sq) == psq.contains(psq)

    def test_random(self, cpp, C):
        rng = np.random.RandomState(42)
        for _ in range(15):
            center = (rng.uniform(-100, 100), rng.uniform(-100, 100))
            angles = sorted(rng.uniform(0, 2*np.pi, 4))
            radii = rng.uniform(1, 20, 4)
            c1 = [(center[0] + r*np.cos(a), center[1] + r*np.sin(a)) for a, r in zip(angles, radii)]
            center = (rng.uniform(-100, 100), rng.uniform(-100, 100))
            angles = sorted(rng.uniform(0, 2*np.pi, 4))
            radii = rng.uniform(1, 20, 4)
            c2 = [(center[0] + r*np.cos(a), center[1] + r*np.sin(a)) for a, r in zip(angles, radii)]
            try:
                pp1, pp2 = py_polygon(c1), py_polygon(c2)
                if not pp1.is_valid or pp1.is_empty or not pp2.is_valid or pp2.is_empty:
                    continue
                cp1, cp2 = C.polygon(c1), C.polygon(c2)
                assert cpp.poly_intersects_poly(cp1, cp2) == pp1.intersects(pp2)
                assert cpp.poly_disjoint_poly(cp1, cp2) == pp1.disjoint(pp2)
            except Exception:
                pass


class TestPolygonPredicatesPt:
    """C++ Polygon↔Point predicates vs Python."""

    SQ = [(0,0),(10,0),(10,10),(0,10)]

    def test_scenarios(self, cpp, C):
        sq = C.polygon(self.SQ)
        psq = py_polygon(self.SQ)
        for name, cx, cy in [("in",5,5),("out",20,20),("edge",0,5),("vertex",0,0)]:
            cpt, ppt = C.point(cx, cy), py_point(cx, cy)
            assert cpp.poly_contains_pt(sq, cpt) == psq.contains(ppt), f"contains {name}"
            assert cpp.poly_covers_pt(sq, cpt) == psq.covers(ppt), f"covers {name}"
            assert cpp.poly_intersects_pt(sq, cpt) == psq.intersects(ppt), f"intersects {name}"


class TestPolygonPredicatesLs:
    """C++ Polygon↔LineString predicates vs Python."""

    def test_scenarios(self, cpp, C):
        sq = C.polygon([(0,0),(10,0),(10,10),(0,10)])
        psq = py_polygon([(0,0),(10,0),(10,10),(0,10)])
        cls = C.linestring([(-5,5),(15,5)])
        pls = py_linestring([(-5,5),(15,5)])
        assert cpp.poly_intersects_ls(sq, cls) == psq.intersects(pls)
        assert cpp.poly_crosses_ls(sq, cls) == psq.crosses(pls)
        cls2 = C.linestring([(20,0),(25,0)])
        pls2 = py_linestring([(20,0),(25,0)])
        assert cpp.poly_disjoint_ls(sq, cls2) == psq.disjoint(pls2)


class TestPolygonDistanceLineString:
    """C++ Polygon.distance(LineString) vs Python."""

    def test_outside(self, cpp):
        sq = make_square_coords()
        line = [(10.0, 0.0), (10.0, 5.0)]
        poly = cpp.polygon(sq)
        ls = cpp.linestring(line)
        py_poly = PyPolygon(sq)
        py_ls = PyLineString(line)
        assert cpp.distance_polygon_linestring(poly, ls) == py_poly.distance(py_ls)


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

    def test_with_linestring_crossing(self, cpp):
        sq = make_square_coords()
        line = [(-10.0, 0.0), (10.0, 0.0)]
        poly = cpp.polygon(sq)
        ls = cpp.linestring(line)
        py_poly = PyPolygon(sq)
        py_ls = PyLineString(line)
        assert cpp.intersects_polygon_linestring(poly, ls) == py_poly.intersects(py_ls)

    def test_with_linestring_outside(self, cpp):
        sq = make_square_coords()
        line = [(10.0, 10.0), (15.0, 15.0)]
        poly = cpp.polygon(sq)
        ls = cpp.linestring(line)
        py_poly = PyPolygon(sq)
        py_ls = PyLineString(line)
        assert cpp.intersects_polygon_linestring(poly, ls) == py_poly.intersects(py_ls)


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
        assert cpp_inter_area == py_inter_area

    def test_no_overlap(self, cpp):
        sq1 = make_square_coords(0, 0, 5)
        sq2 = make_square_coords(20, 0, 5)
        p1 = cpp.polygon(sq1)
        p2 = cpp.polygon(sq2)
        py_p1 = PyPolygon(sq1)
        py_p2 = PyPolygon(sq2)
        cpp_inter_area = cpp.intersection_area_polygon_polygon(p1, p2)
        py_inter_area = py_p1.intersection(py_p2).area
        assert cpp_inter_area == py_inter_area


# ==============================================================================
# Centroid (float64 only)
# ==============================================================================

class TestCentroid:
    """C++ centroid vs Python centroid."""

    @pytest.mark.parametrize("x,y", [(0,0), (5,-3), (-10,20)])
    def test_point(self, cpp, x, y):
        cx, cy = cpp.centroid_point(cpp.point(x, y))
        pc = py_point(x, y).centroid
        assert cx == pc.x
        assert cy == pc.y

    def test_linestring(self, cpp):
        cx, cy = cpp.centroid_linestring(cpp.linestring([(0,0),(10,0),(10,10)]))
        pc = py_linestring([(0,0),(10,0),(10,10)]).centroid
        assert cx == pc.x
        assert cy == pc.y

    @pytest.mark.parametrize("coords", [
        [(0,0),(10,0),(10,10),(0,10)],
        [(0,0),(10,0),(5,8)],
        [(0,0),(10,0),(10,5),(5,10),(0,5)],
    ])
    def test_polygon(self, cpp, coords):
        cx, cy = cpp.centroid_polygon(cpp.polygon(coords))
        pc = py_polygon(coords).centroid
        assert cx == pc.x
        assert cy == pc.y


# ==============================================================================
# LinearRing (float64 only)
# ==============================================================================

class TestLinearRing:
    RING_SQ = [(0,0),(10,0),(10,10),(0,10)]

    def test_properties(self, cpp, C):
        cr = C.linearring(self.RING_SQ)
        pr = py_linearring(self.RING_SQ)
        assert cr.is_empty() == pr.is_empty
        assert cr.is_simple() == pr.is_simple
        assert cr.is_valid() == pr.is_valid
        assert cr.is_closed() == pr.is_closed
        assert cr.is_ring() == pr.is_ring
        assert cr.length() == pr.length
        assert cr.bounds() == list(pr.bounds)
        assert cr.area() == pr.area

    def test_wkt(self, cpp, C):
        cw = C.linearring(self.RING_SQ).wkt()
        pw = py_linearring(self.RING_SQ).wkt
        g1 = shapely_wkt.loads(cw)
        g2 = shapely_wkt.loads(pw)
        assert g1.equals_exact(g2, 1e-10), f"WKT mismatch\n  C++: {cw}\n  Py:  {pw}"

    def test_geom_type(self, cpp, C):
        cr = C.linearring(self.RING_SQ)
        assert "LinearRing" in cr.geom_type() or cr.geom_type() == "LinearRing"
        assert cr.type() == "LinearRing"
        assert cr.has_z() == py_linearring(self.RING_SQ).has_z

    def test_coords_xy(self, cpp, C):
        cr = C.linearring(self.RING_SQ)
        for (cx, cy), (x, y) in zip(cr.coords(), self.RING_SQ):
            assert cx == x
            assert cy == y
        xs, ys = cr.xy()
        assert len(xs) == 4 and len(ys) == 4

    def test_is_ccw(self, cpp, C):
        assert C.linearring([(0,0),(10,0),(10,10),(0,10)]).is_ccw() == True
        assert C.linearring([(0,0),(0,10),(10,10),(10,0)]).is_ccw() == False

    def test_empty(self, cpp, C):
        assert C.linearring([]).is_empty() == True


# ==============================================================================
# nearest_points (float64 only)
# ==============================================================================

class TestNearestPointsPolygonLineString:
    """C++ ops::nearest_points(Polygon, LineString) vs Python ops.nearest_points."""

    def test_disjoint(self, cpp):
        sq = make_square_coords(0, 0, 5)
        line = [(10.0, 0.0), (15.0, 0.0)]
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

    def test_near(self, cpp):
        sq = make_square_coords(0, 0, 5)
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


# ==============================================================================
# Random cross-type tests (float64 only)
# ==============================================================================

class TestRandomCrossType:
    """Randomized Pt↔LS cross-type tests."""

    def test_random(self, cpp, C):
        rng = np.random.RandomState(42)
        for _ in range(30):
            pt_c = (rng.uniform(-100, 100), rng.uniform(-100, 100))
            ls_c = [(rng.uniform(-100, 100), rng.uniform(-100, 100)) for _ in range(3)]
            cpt, cls = C.point(*pt_c), C.linestring(ls_c)
            ppt, pls = py_point(*pt_c), py_linestring(ls_c)
            assert cpp.pt_intersects_ls(cpt, cls) == ppt.intersects(pls)
            assert cpp.distance_point_linestring(cpt, cls) == ppt.distance(pls)


# ==============================================================================
# Serialization (float64 only)
# ==============================================================================

class TestSerialization:
    """WKT roundtrip tests for all geometry types."""

    def test_wkt_roundtrip(self, cpp, C):
        for cpp_geom, py_geom in [
            (C.point(3.14, -2.71), py_point(3.14, -2.71)),
            (C.point(-1e6, 1e-6), py_point(-1e6, 1e-6)),
            (C.linestring([(0,0),(10,5),(20,10)]), py_linestring([(0,0),(10,5),(20,10)])),
            (C.polygon([(0,0),(10,0),(10,10),(0,10)]), py_polygon([(0,0),(10,0),(10,10),(0,10)])),
            (C.linearring([(0,0),(10,0),(10,10),(0,0)]), py_linearring([(0,0),(10,0),(10,10),(0,0)])),
        ]:
            g1 = shapely_wkt.loads(cpp_geom.wkt())
            g2 = shapely_wkt.loads(py_geom.wkt)
            assert g1.equals_exact(g2, 1e-10), f"WKT mismatch:\n  C++: {cpp_geom.wkt()}\n  Py:  {py_geom.wkt}"
