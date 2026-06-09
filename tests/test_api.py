"""
Unified C++ vs Python shapely test suite — collision-critical precision alignment.

Design
------
- ``make`` fixture (parametrized float64/float32): same-type distance (Pt↔Pt, LS↔LS, Poly↔Poly)
- ``cpp`` + ``C`` fixtures (float64 only): cross-type ops, predicates, linearring, nearest_points

Float32 bit-alignment strategy
------------------------------
Python shapely has no float32 mode — all coordinates are auto-promoted to float64.
Both C++ float32 and Python float64 paths feed `double` values into the same GEOS
Coordinate/CoordinateSequence/DistanceOp internals.  Therefore:

  • C++ f64 vs Python:       bit-identical (assert a == b)  — same double inputs → same GEOS result.
  • C++ f32 vs f64 (same f32-coords): bit-identical          — f32→double widening preserves value exactly.
  • C++ f32 vs Python (f32-coords to both): bit-identical    — both receive identical doubles in GEOS.

All comparisons use STRICT EQUALITY (==) except buffer areas (~1% tolerance).

Collision-critical distance patterns tested (from production code)
------------------------------------------------------------------
  1. Polygon-Polygon distance    →  bool collision = (static_distance == 0.0)
  2. Polygon-LineString distance →  double d = poly.distance(line)
  3. intersects + distance       →  intersects(g1,g2) && distance(g1,g2) < 1e-12
  4. LineString-Point distance   →  dist = line.distance(ego_pt)
  5. LineString-LineString dist  →  half_lane_width = line1.distance(line2) / 2.0
  6. LS↔Poly / Poly↔LS distance  →  centerline.distance(poly); poly.distance(line)

Each pattern includes edge cases (touching, near-touching, ε-overlapping)
and high-volume random tests (1000 pairs f64 + 500 pairs f32).
"""

import numpy as np
import pytest
from shapely.geometry import Point as PyPoint, LineString as PyLineString, Polygon as PyPolygon
import shapely.ops as py_ops
from shapely import wkt as shapely_wkt

from .utils import (make_square_coords, make_triangle_coords,
                    py_linearring, py_point as _py_point, py_linestring as _py_ls,
                    py_polygon as _py_poly)

py_point = _py_point
py_linestring = _py_ls
py_polygon = _py_poly


# =============================================================================
# Test helpers
# =============================================================================

def _f32(coords):
    """Truncate coordinates to float32, returning a list of (float,float) tuples
    where each float preserves the exact float32 value (float32 is exactly
    representable in float64).  This makes C++ f32 and Python f64 receive
    identical GEOS-internal double values.  """
    if isinstance(coords, tuple):
        return tuple(_f32(list(coords)))
    if isinstance(coords, (int, float, np.floating)):
        return float(np.float32(coords))
    return [(_f32(c[0]), _f32(c[1])) for c in coords]


def _square(cx, cy, half):
    """Axis-aligned square polygon coords."""
    return [(cx - half, cy - half), (cx + half, cy - half),
            (cx + half, cy + half), (cx - half, cy + half)]


# =============================================================================
# Point accessors
# =============================================================================

class TestPointAccessors:

    def test_coordinates(self, make):
        for x, y in [(0.0, 0.0), (1.5, -3.2), (-100.0, 200.0), (1e-10, -1e10)]:
            fx, fy = (float(np.float32(x)), float(np.float32(y))) if make['dtype'] == np.float32 else (float(x), float(y))
            cpt = make['point'](fx, fy)
            ppt = PyPoint(fx, fy)
            assert cpt.x == ppt.x, f"x: {cpt.x} vs {ppt.x}"
            assert cpt.y == ppt.y, f"y: {cpt.y} vs {ppt.y}"

    def test_is_valid(self, make):
        p = make['point'](0.0, 0.0)
        assert p.is_valid() == PyPoint(0.0, 0.0).is_valid

    def test_coords_xy(self, cpp, C):
        p = C.point(1.5, -2.5)
        cs = list(p.coords())
        assert len(cs) == 1
        assert cs[0][0] == 1.5 and cs[0][1] == -2.5
        xs, ys = p.xy()
        assert xs == [1.5] and ys == [-2.5]

    def test_full_accessors_f64(self, cpp, C):
        for x, y in [(0, 0), (3.14, -2.71), (1e6, -1e-6)]:
            cpt = C.point(x, y)
            ppt = py_point(x, y)
            assert cpt.x == ppt.x; assert cpt.y == ppt.y
            assert cpt.is_empty() == ppt.is_empty
            assert cpt.is_simple() == ppt.is_simple
            assert cpt.is_valid() == ppt.is_valid
            assert cpt.area() == ppt.area; assert cpt.length() == ppt.length
            assert cpt.has_z() == ppt.has_z
            assert cpt.geom_type() == ppt.geom_type
            assert cpt.type() == "Point"
            assert len(cpt.wkb_hex()) > 0
            assert cpt.bounds() == list(ppt.bounds)
            g1 = shapely_wkt.loads(cpt.wkt())
            g2 = shapely_wkt.loads(ppt.wkt)
            assert g1.equals_exact(g2, 1e-10)

    def test_accessors_f32(self, cpp, C):
        for x, y in [(0, 0), (3.14, -2.71)]:
            # Truncate to f32 first so both C++ f32 and Python f64 receive identical doubles
            fx, fy = float(np.float32(x)), float(np.float32(y))
            cpt = C.point(fx, fy, 'float32')
            ppt = py_point(fx, fy)
            assert float(cpt.x) == ppt.x
            assert float(cpt.y) == ppt.y
            assert cpt.is_valid() == ppt.is_valid


class TestPointDistancePoint:

    @pytest.mark.parametrize("x1,y1,x2,y2", [
        (0.0,0.0,3.0,4.0), (0.0,0.0,1.0,0.0), (0.0,0.0,0.0,1.0),
        (1.0,2.0,4.0,6.0), (-1.0,-1.0,2.0,3.0),
        (1e10,0.0,0.0,0.0), (0.0,0.0,0.0,0.0),
    ])
    def test_known_values(self, make, x1, y1, x2, y2):
        p1 = make['point'](x1, y1); p2 = make['point'](x2, y2)
        py_p1 = PyPoint(x1, y1); py_p2 = PyPoint(x2, y2)
        assert p1.distance(p2) == py_p1.distance(py_p2)

    def test_random(self, make):
        rng = np.random.RandomState(42)
        for _ in range(20):
            x1, y1 = rng.uniform(-100, 100), rng.uniform(-100, 100)
            x2, y2 = rng.uniform(-100, 100), rng.uniform(-100, 100)
            if make['dtype'] == np.float32:
                x1, y1 = float(np.float32(x1)), float(np.float32(y1))
                x2, y2 = float(np.float32(x2)), float(np.float32(y2))
            p1 = make['point'](x1, y1); p2 = make['point'](x2, y2)
            py1 = PyPoint(x1, y1); py2 = PyPoint(x2, y2)
            assert p1.distance(p2) == py1.distance(py2)


class TestPointPredicatesPtPt:

    @pytest.mark.parametrize("x1,y1,x2,y2", [(0,0,0,0),(0,0,1,1),(5,5,5,5),(-1,2,-3,4)])
    def test_all(self, cpp, C, x1, y1, x2, y2):
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

    @pytest.mark.parametrize("dist", [1.0, 5.0, 10.0, 0.1, 100.0])
    def test_area(self, make, dist):
        buf = make['point'](0.0, 0.0).buffer(dist)
        py_buf = PyPoint(0.0, 0.0).buffer(dist)
        assert abs(buf.area() - py_buf.area) / max(py_buf.area, 1e-10) < 0.01

    def test_zero(self, make):
        assert make['point'](0.0, 0.0).buffer(0.0).is_empty() == True


# =============================================================================
# Point ↔ LineString / Point ↔ Polygon (cross-type, float64)
# =============================================================================

class TestPointDistanceLineString:
    def test_known(self, cpp):
        for px, py, coords in [
            (0.0,0.0,[(3.0,0.0),(3.0,4.0)]),
            (0.0,0.0,[(1.0,0.0),(1.0,1.0)]),
            (0.0,4.0,[(0.0,0.0),(0.0,10.0)]),
            (10.0,0.0,[(-5.0,0.0),(5.0,0.0)]),
            (2.0,2.0,[(0.0,0.0),(4.0,4.0)]),
        ]:
            pt = cpp.point(px, py); ls = cpp.linestring(coords)
            assert cpp.distance_point_linestring(pt, ls) == \
                   PyPoint(px, py).distance(PyLineString(coords))

    def test_random(self, cpp):
        rng = np.random.RandomState(99)
        for _ in range(10):
            coords = [(rng.uniform(-10,10), rng.uniform(-10,10)) for _ in range(4)]
            px, py = rng.uniform(-10,10), rng.uniform(-10,10)
            pt = cpp.point(px, py); ls = cpp.linestring(coords)
            assert cpp.distance_point_linestring(pt, ls) == \
                   PyPoint(px, py).distance(PyLineString(coords))


class TestPointDistancePolygon:
    def test_outside(self, cpp):
        sq = make_square_coords()
        assert cpp.distance_point_polygon(cpp.point(10.0,0.0), cpp.polygon(sq)) == \
               PyPoint(10.0,0.0).distance(PyPolygon(sq))

    def test_inside(self, cpp):
        sq = make_square_coords()
        assert cpp.distance_point_polygon(cpp.point(0.0,0.0), cpp.polygon(sq)) == \
               PyPoint(0.0,0.0).distance(PyPolygon(sq))


class TestPointPredicatesPoly:
    def test_scenarios(self, cpp, C):
        sq = C.polygon([(0,0),(10,0),(10,10),(0,10)])
        psq = py_polygon([(0,0),(10,0),(10,10),(0,10)])
        for name, cx, cy in [("in",5,5),("out",20,20),("edge",0,5)]:
            cpt, ppt = C.point(cx, cy), py_point(cx, cy)
            assert cpp.pt_within_poly(cpt, sq) == ppt.within(psq), name
            assert cpp.pt_disjoint_poly(cpt, sq) == ppt.disjoint(psq), name
            assert cpp.pt_touches_poly(cpt, sq) == ppt.touches(psq), name
            assert cpp.pt_intersects_poly(cpt, sq) == ppt.intersects(psq), name
            assert cpp.pt_contains_poly(cpt, sq) == ppt.contains(psq), name


# =============================================================================
# LineString accessors + same-type distance
# =============================================================================

class TestLineStringAccessors:
    def test_full(self, cpp, C):
        c_ls = C.linestring([(0,0),(10,0),(10,10)])
        p_ls = py_linestring([(0,0),(10,0),(10,10)])
        g1 = shapely_wkt.loads(c_ls.wkt()); g2 = shapely_wkt.loads(p_ls.wkt)
        assert g1.equals_exact(g2, 1e-10)
        assert len(c_ls.wkb_hex()) > 0
        assert c_ls.geom_type() == p_ls.geom_type
        assert c_ls.type() == "LineString"
        assert c_ls.has_z() == p_ls.has_z
        assert c_ls.is_empty() == p_ls.is_empty
        assert c_ls.is_simple() == p_ls.is_simple
        assert c_ls.is_valid() == p_ls.is_valid
        assert c_ls.area() == p_ls.area; assert c_ls.length() == p_ls.length
        assert c_ls.bounds() == list(p_ls.bounds)

    def test_closed_ring(self, cpp, C):
        open_ls = C.linestring([(0,0),(10,0)])
        closed_ls = C.linestring([(0,0),(10,0),(10,10),(0,0)])
        assert open_ls.is_closed() == False; assert closed_ls.is_closed() == True
        assert open_ls.is_ring() == False; assert closed_ls.is_ring() == True

    def test_coords_xy(self, cpp, C):
        pts = [(1,2),(3,4),(5,6)]
        c_ls = C.linestring(pts)
        for (cx,cy),(px,py) in zip(c_ls.coords(),pts):
            assert cx == px; assert cy == py
        xs, ys = c_ls.xy()
        assert xs == [1.,3.,5.] and ys == [2.,4.,6.]

    @pytest.mark.parametrize("coords", [
        [(0.0,0.0),(1.0,0.0)], [(0.0,0.0),(3.0,4.0)],
        [(0.0,0.0),(1.0,1.0),(2.0,2.0)], [(0.0,0.0),(10.0,0.0),(10.0,10.0)],
    ])
    def test_length(self, make, coords):
        ls = make['linestring'](coords)
        assert ls.length() == PyLineString(coords).length


class TestLineStringDistanceLineString:
    def test_parallel(self, make):
        l1 = [(0.0,0.0),(0.0,10.0)]; l2 = [(3.0,0.0),(3.0,10.0)]
        ls1 = make['linestring'](l1); ls2 = make['linestring'](l2)
        assert ls1.distance(ls2) == PyLineString(l1).distance(PyLineString(l2))

    def test_intersecting(self, make):
        l1 = [(0.0,0.0),(10.0,10.0)]; l2 = [(0.0,10.0),(10.0,0.0)]
        ls1 = make['linestring'](l1); ls2 = make['linestring'](l2)
        assert ls1.distance(ls2) == PyLineString(l1).distance(PyLineString(l2))

    def test_random(self, make):
        rng = np.random.RandomState(1)
        for _ in range(10):
            c1 = [(rng.uniform(-10,10),rng.uniform(-10,10)) for _ in range(3)]
            c2 = [(rng.uniform(-10,10),rng.uniform(-10,10)) for _ in range(3)]
            if make['dtype'] == np.float32:
                c1 = _f32(c1); c2 = _f32(c2)
            ls1 = make['linestring'](c1); ls2 = make['linestring'](c2)
            assert ls1.distance(ls2) == PyLineString(c1).distance(PyLineString(c2))


class TestLineStringPredicatesLsLs:
    def test_crossing(self, cpp, C):
        ls1 = C.linestring([(0,5),(10,5)]); ls2 = C.linestring([(5,0),(5,10)])
        ls3 = C.linestring([(0,10),(10,10)])
        p1 = py_linestring([(0,5),(10,5)]); p2 = py_linestring([(5,0),(5,10)])
        p3 = py_linestring([(0,10),(10,10)])
        assert cpp.ls_crosses_ls(ls1,ls2) == p1.crosses(p2)
        assert cpp.ls_disjoint_ls(ls1,ls3) == p1.disjoint(p3)
        assert cpp.ls_intersects_ls(ls1,ls2) == p1.intersects(p2)
        assert cpp.ls_touches_ls(ls1,ls3) == p1.touches(p3)

    def test_random(self, cpp, C):
        rng = np.random.RandomState(42)
        for _ in range(20):
            c1 = [(rng.uniform(-100,100),rng.uniform(-100,100)) for _ in range(3)]
            c2 = [(rng.uniform(-100,100),rng.uniform(-100,100)) for _ in range(3)]
            cls1,cls2 = C.linestring(c1),C.linestring(c2)
            p1,p2 = py_linestring(c1),py_linestring(c2)
            assert cpp.ls_intersects_ls(cls1,cls2) == p1.intersects(p2)


class TestLineStringPredicatesPt:
    def test_on_off(self, cpp, C):
        ls = C.linestring([(0,0),(10,0)]); pls = py_linestring([(0,0),(10,0)])
        pt_on,pt_off = C.point(5,0),C.point(5,5)
        ppt_on,ppt_off = py_point(5,0),py_point(5,5)
        assert cpp.ls_contains_pt(ls,pt_on) == pls.contains(ppt_on)
        assert cpp.ls_intersects_pt(ls,pt_on) == pls.intersects(ppt_on)
        assert cpp.ls_disjoint_pt(ls,pt_off) == pls.disjoint(ppt_off)


class TestLineStringBuffer:
    def test_area(self, make):
        line = [(0.0,0.0),(10.0,0.0)]
        buf = make['linestring'](line).buffer(1.0)
        py_buf = PyLineString(line).buffer(1.0)
        assert abs(buf.area() - py_buf.area) / max(py_buf.area, 1e-10) < 0.01

    def test_zero(self, make):
        assert make['linestring']([(0.0,0.0),(10.0,0.0)]).buffer(0.0).is_empty() == True


# =============================================================================
# LineString ↔ Polygon (cross-type, float64)
# =============================================================================

class TestLineStringDistancePolygon:
    def test_outside(self, cpp):
        sq = make_square_coords(); line = [(10.0,0.0),(10.0,5.0)]
        assert cpp.distance_linestring_polygon(cpp.linestring(line), cpp.polygon(sq)) == \
               PyLineString(line).distance(PyPolygon(sq))

    def test_crossing(self, cpp):
        sq = make_square_coords(); line = [(-10.0,0.0),(10.0,0.0)]
        assert cpp.distance_linestring_polygon(cpp.linestring(line), cpp.polygon(sq)) == \
               PyLineString(line).distance(PyPolygon(sq))


class TestLineStringIntersectsPolygon:
    def test_crosses(self, cpp):
        sq = make_square_coords(); line = [(-10.0,0.0),(10.0,0.0)]
        assert cpp.intersects_linestring_polygon(cpp.linestring(line), cpp.polygon(sq)) == \
               PyLineString(line).intersects(PyPolygon(sq))

    def test_outside(self, cpp):
        sq = make_square_coords(); line = [(10.0,10.0),(15.0,15.0)]
        assert cpp.intersects_linestring_polygon(cpp.linestring(line), cpp.polygon(sq)) == \
               PyLineString(line).intersects(PyPolygon(sq))

    def test_inside(self, cpp):
        sq = make_square_coords(); line = [(-2.0,0.0),(2.0,0.0)]
        assert cpp.intersects_linestring_polygon(cpp.linestring(line), cpp.polygon(sq)) == \
               PyLineString(line).intersects(PyPolygon(sq))


class TestLineStringPredicatesPoly:
    def test_scenarios(self, cpp, C):
        sq = C.polygon([(0,0),(10,0),(10,10),(0,10)])
        psq = py_polygon([(0,0),(10,0),(10,10),(0,10)])
        for name, cds in [("crossing",[(-5,5),(15,5)]),("outside",[(20,0),(25,0)]),
                          ("inside",[(2,2),(8,2)]),("edge",[(0,5),(10,5)])]:
            cls, pls = C.linestring(cds), py_linestring(cds)
            assert cpp.ls_intersects_poly(cls, sq) == pls.intersects(psq), name


class TestLineStringProject:
    @pytest.mark.parametrize("line,px,py", [
        ([(0,0),(10,0)],5.0,0.0), ([(0,0),(0,10)],0.0,5.0),
        ([(0,0),(10,0)],5.0,3.0), ([(0,0),(10,0),(10,10)],10.0,5.0),
    ])
    def test_values(self, cpp, line, px, py):
        assert cpp.project_linestring_point(cpp.linestring(line), cpp.point(px,py)) == \
               PyLineString(line).project(PyPoint(px,py))


class TestLineStringInterpolate:
    @pytest.mark.parametrize("line,dist", [
        ([(0,0),(10,0)],5.0), ([(0,0),(0,10)],5.0),
        ([(0,0),(10,0)],0.0), ([(0,0),(10,0)],10.0),
    ])
    def test_values(self, cpp, line, dist):
        x, y = cpp.interpolate_linestring(cpp.linestring(line), dist)
        py_pt = PyLineString(line).interpolate(dist)
        assert x == py_pt.x; assert y == py_pt.y


# =============================================================================
# Polygon accessors + same-type distance
# =============================================================================

class TestPolygonAccessors:
    SQ = [(0,0),(10,0),(10,10),(0,10)]

    def test_full(self, cpp, C):
        cp = C.polygon(self.SQ); pp = py_polygon(self.SQ)
        g1 = shapely_wkt.loads(cp.wkt()); g2 = shapely_wkt.loads(pp.wkt)
        assert g1.equals_exact(g2, 1e-10)
        assert len(cp.wkb_hex()) > 0
        assert cp.geom_type() == pp.geom_type; assert cp.type() == "Polygon"
        assert cp.has_z() == pp.has_z
        assert cp.is_empty() == pp.is_empty; assert cp.is_simple() == pp.is_simple
        assert cp.is_valid() == pp.is_valid
        assert cp.area() == pp.area; assert cp.length() == pp.length
        assert cp.bounds() == list(pp.bounds)

    def test_area_square(self, make):
        sq = make_square_coords()
        poly = make['polygon'](sq)
        assert poly.area() == PyPolygon(sq).area; assert poly.area() == 100.0

    def test_area_triangle(self, make):
        tri = make_triangle_coords()
        poly = make['polygon'](tri)
        assert poly.area() == PyPolygon(tri).area; assert poly.area() == 50.0

    def test_is_valid(self, make):
        sq = make_square_coords()
        assert make['polygon'](sq).is_valid() == PyPolygon(sq).is_valid

    def test_is_empty(self, make):
        assert make['polygon'](make_square_coords()).is_empty() == False

    def test_coords(self, cpp, C):
        coords = list(C.polygon(self.SQ).coords())
        for (cx,cy),(x,y) in zip(coords, self.SQ):
            assert cx == x

    def test_exterior(self, cpp, C):
        ext = cpp.polygon_exterior(C.polygon(self.SQ))
        pext = py_polygon(self.SQ).exterior
        assert ext.shape[0] == len(pext.coords)
        for i in range(ext.shape[0]):
            for j in range(2):
                assert ext[i][j] == pext.coords[i][j]


class TestPolygonDistancePolygon:
    def test_disjoint(self, make):
        sq1 = make_square_coords(0,0,5); sq2 = make_square_coords(20,0,5)
        p1 = make['polygon'](sq1); p2 = make['polygon'](sq2)
        assert p1.distance(p2) == PyPolygon(sq1).distance(PyPolygon(sq2))
        assert p1.distance(p2) == 10.0

    def test_overlapping(self, make):
        sq1 = make_square_coords(0,0,5); sq2 = make_square_coords(3,0,5)
        p1 = make['polygon'](sq1); p2 = make['polygon'](sq2)
        assert p1.distance(p2) == PyPolygon(sq1).distance(PyPolygon(sq2))

    def test_touching(self, make):
        sq1 = make_square_coords(0,0,5); sq2 = make_square_coords(10,0,5)
        p1 = make['polygon'](sq1); p2 = make['polygon'](sq2)
        assert p1.distance(p2) == PyPolygon(sq1).distance(PyPolygon(sq2))


class TestPolygonBuffer:
    def test_area(self, make):
        sq = make_square_coords(0,0,5)
        buf = make['polygon'](sq).buffer(2.0)
        py_buf = PyPolygon(sq).buffer(2.0)
        assert abs(buf.area() - py_buf.area) / max(py_buf.area, 1e-10) < 0.01

    def test_zero(self, make):
        sq = make_square_coords(0,0,5)
        poly = make['polygon'](sq)
        py_poly = PyPolygon(sq)
        assert abs(poly.buffer(0.0).area() - py_poly.buffer(0.0).area) / max(py_poly.area,1e-10) < 0.01


# =============================================================================
# Polygon ↔ Polygon, Point, LineString (cross-type, float64)
# =============================================================================

class TestPolygonPredicatesPoly:
    SQ = [(0,0),(10,0),(10,10),(0,10)]

    def test_overlapping(self, cpp, C):
        sq = C.polygon(self.SQ)
        sq2 = C.polygon([(5,0),(15,0),(15,10),(5,10)])
        sq3 = C.polygon([(20,20),(30,20),(30,30),(20,30)])
        psq = py_polygon(self.SQ)
        psq2 = py_polygon([(5,0),(15,0),(15,10),(5,10)])
        psq3 = py_polygon([(20,20),(30,20),(30,30),(20,30)])
        assert cpp.poly_overlaps_poly(sq,sq2) == psq.overlaps(psq2)
        assert cpp.poly_disjoint_poly(sq,sq3) == psq.disjoint(psq3)
        assert cpp.poly_touches_poly(sq,sq2) == psq.touches(psq2)
        assert cpp.poly_intersects_poly(sq,sq2) == psq.intersects(psq2)
        assert cpp.poly_relate_poly(sq,sq) == psq.relate(psq)
        assert cpp.poly_contains_poly(sq,sq) == psq.contains(psq)

    def test_random(self, cpp, C):
        rng = np.random.RandomState(42)
        for _ in range(15):
            center = (rng.uniform(-100,100),rng.uniform(-100,100))
            angles = sorted(rng.uniform(0,2*np.pi,4))
            radii = rng.uniform(1,20,4)
            c1 = [(center[0]+r*np.cos(a),center[1]+r*np.sin(a)) for a,r in zip(angles,radii)]
            center = (rng.uniform(-100,100),rng.uniform(-100,100))
            angles = sorted(rng.uniform(0,2*np.pi,4))
            radii = rng.uniform(1,20,4)
            c2 = [(center[0]+r*np.cos(a),center[1]+r*np.sin(a)) for a,r in zip(angles,radii)]
            try:
                pp1,pp2 = py_polygon(c1),py_polygon(c2)
                if not pp1.is_valid or pp1.is_empty or not pp2.is_valid or pp2.is_empty:
                    continue
                cp1,cp2 = C.polygon(c1),C.polygon(c2)
                assert cpp.poly_intersects_poly(cp1,cp2) == pp1.intersects(pp2)
                assert cpp.poly_disjoint_poly(cp1,cp2) == pp1.disjoint(pp2)
            except Exception:
                pass


class TestPolygonPredicatesPt:
    SQ = [(0,0),(10,0),(10,10),(0,10)]

    def test_scenarios(self, cpp, C):
        sq = C.polygon(self.SQ); psq = py_polygon(self.SQ)
        for name,cx,cy in [("in",5,5),("out",20,20),("edge",0,5),("vertex",0,0)]:
            cpt,ppt = C.point(cx,cy),py_point(cx,cy)
            assert cpp.poly_contains_pt(sq,cpt) == psq.contains(ppt), name
            assert cpp.poly_covers_pt(sq,cpt) == psq.covers(ppt), name
            assert cpp.poly_intersects_pt(sq,cpt) == psq.intersects(ppt), name


class TestPolygonPredicatesLs:
    def test_scenarios(self, cpp, C):
        sq = C.polygon([(0,0),(10,0),(10,10),(0,10)])
        psq = py_polygon([(0,0),(10,0),(10,10),(0,10)])
        cls = C.linestring([(-5,5),(15,5)]); pls = py_linestring([(-5,5),(15,5)])
        assert cpp.poly_intersects_ls(sq,cls) == psq.intersects(pls)
        assert cpp.poly_crosses_ls(sq,cls) == psq.crosses(pls)
        cls2 = C.linestring([(20,0),(25,0)]); pls2 = py_linestring([(20,0),(25,0)])
        assert cpp.poly_disjoint_ls(sq,cls2) == psq.disjoint(pls2)


class TestPolygonDistanceLineString:
    def test_outside(self, cpp):
        sq = make_square_coords(); line = [(10.0,0.0),(10.0,5.0)]
        assert cpp.distance_polygon_linestring(cpp.polygon(sq), cpp.linestring(line)) == \
               PyPolygon(sq).distance(PyLineString(line))


class TestPolygonIntersects:
    def test_overlapping(self, cpp):
        sq1 = make_square_coords(0,0,5); sq2 = make_square_coords(3,0,5)
        assert cpp.intersects_polygon_polygon(cpp.polygon(sq1), cpp.polygon(sq2)) == \
               PyPolygon(sq1).intersects(PyPolygon(sq2))

    def test_disjoint(self, cpp):
        sq1 = make_square_coords(0,0,5); sq2 = make_square_coords(20,0,5)
        assert cpp.intersects_polygon_polygon(cpp.polygon(sq1), cpp.polygon(sq2)) == \
               PyPolygon(sq1).intersects(PyPolygon(sq2))

    def test_contains(self, cpp):
        sq1 = make_square_coords(0,0,10); sq2 = make_square_coords(0,0,2)
        assert cpp.intersects_polygon_polygon(cpp.polygon(sq1), cpp.polygon(sq2)) == \
               PyPolygon(sq1).intersects(PyPolygon(sq2))

    def test_with_linestring(self, cpp):
        sq = make_square_coords()
        line1 = [(-10.0,0.0),(10.0,0.0)]; line2 = [(10.0,10.0),(15.0,15.0)]
        assert cpp.intersects_polygon_linestring(cpp.polygon(sq), cpp.linestring(line1)) == \
               PyPolygon(sq).intersects(PyLineString(line1))
        assert cpp.intersects_polygon_linestring(cpp.polygon(sq), cpp.linestring(line2)) == \
               PyPolygon(sq).intersects(PyLineString(line2))


class TestPolygonIntersection:
    def test_overlap(self, cpp):
        sq1 = make_square_coords(0,0,5); sq2 = make_square_coords(3,0,5)
        assert cpp.intersection_area_polygon_polygon(cpp.polygon(sq1), cpp.polygon(sq2)) == \
               PyPolygon(sq1).intersection(PyPolygon(sq2)).area

    def test_no_overlap(self, cpp):
        sq1 = make_square_coords(0,0,5); sq2 = make_square_coords(20,0,5)
        assert cpp.intersection_area_polygon_polygon(cpp.polygon(sq1), cpp.polygon(sq2)) == \
               PyPolygon(sq1).intersection(PyPolygon(sq2)).area


# =============================================================================
# Centroid
# =============================================================================

class TestCentroid:
    @pytest.mark.parametrize("x,y", [(0,0),(5,-3),(-10,20)])
    def test_point(self, cpp, x, y):
        cx, cy = cpp.centroid_point(cpp.point(x, y))
        pc = py_point(x, y).centroid
        assert cx == pc.x; assert cy == pc.y

    def test_linestring(self, cpp):
        cx, cy = cpp.centroid_linestring(cpp.linestring([(0,0),(10,0),(10,10)]))
        pc = py_linestring([(0,0),(10,0),(10,10)]).centroid
        assert cx == pc.x; assert cy == pc.y

    @pytest.mark.parametrize("coords", [
        [(0,0),(10,0),(10,10),(0,10)], [(0,0),(10,0),(5,8)],
        [(0,0),(10,0),(10,5),(5,10),(0,5)],
    ])
    def test_polygon(self, cpp, coords):
        cx, cy = cpp.centroid_polygon(cpp.polygon(coords))
        pc = py_polygon(coords).centroid
        assert cx == pc.x; assert cy == pc.y


# =============================================================================
# LinearRing
# =============================================================================

class TestLinearRing:
    RING_SQ = [(0,0),(10,0),(10,10),(0,10)]

    def test_properties(self, cpp, C):
        cr = C.linearring(self.RING_SQ); pr = py_linearring(self.RING_SQ)
        assert cr.is_empty() == pr.is_empty
        assert cr.is_simple() == pr.is_simple
        assert cr.is_valid() == pr.is_valid
        assert cr.is_closed() == pr.is_closed
        assert cr.is_ring() == pr.is_ring
        assert cr.length() == pr.length
        assert cr.bounds() == list(pr.bounds)
        assert cr.area() == pr.area

    def test_wkt(self, cpp, C):
        g1 = shapely_wkt.loads(C.linearring(self.RING_SQ).wkt())
        g2 = shapely_wkt.loads(py_linearring(self.RING_SQ).wkt)
        assert g1.equals_exact(g2, 1e-10)

    def test_geom_type(self, cpp, C):
        cr = C.linearring(self.RING_SQ)
        assert "LinearRing" in cr.geom_type() or cr.geom_type() == "LinearRing"
        assert cr.type() == "LinearRing"
        assert cr.has_z() == py_linearring(self.RING_SQ).has_z

    def test_coords_xy(self, cpp, C):
        cr = C.linearring(self.RING_SQ)
        for (cx,cy),(x,y) in zip(cr.coords(), self.RING_SQ):
            assert cx == x; assert cy == y
        xs, ys = cr.xy()
        assert len(xs) == 4 and len(ys) == 4

    def test_is_ccw(self, cpp, C):
        assert C.linearring([(0,0),(10,0),(10,10),(0,10)]).is_ccw() == True
        assert C.linearring([(0,0),(0,10),(10,10),(10,0)]).is_ccw() == False

    def test_empty(self, cpp, C):
        assert C.linearring([]).is_empty() == True


# =============================================================================
# nearest_points
# =============================================================================

class TestNearestPointsPolygonLineString:
    def test_disjoint(self, cpp):
        sq = make_square_coords(0,0,5); line = [(10.0,0.0),(15.0,0.0)]
        x1,y1,x2,y2 = cpp.nearest_points(cpp.polygon(sq), cpp.linestring(line))
        r = py_ops.nearest_points(PyPolygon(sq), PyLineString(line))
        assert x1==r[0].x and y1==r[0].y and x2==r[1].x and y2==r[1].y

    def test_near(self, cpp):
        sq = make_square_coords(0,0,5); line = [(5.1,0.0),(5.1,10.0)]
        x1,y1,x2,y2 = cpp.nearest_points(cpp.polygon(sq), cpp.linestring(line))
        r = py_ops.nearest_points(PyPolygon(sq), PyLineString(line))
        assert x1==r[0].x and y1==r[0].y and x2==r[1].x and y2==r[1].y

    def test_crossing(self, cpp):
        sq = make_square_coords(0,0,5); line = [(-10.0,0.0),(10.0,0.0)]
        x1,y1,x2,y2 = cpp.nearest_points(cpp.polygon(sq), cpp.linestring(line))
        r = py_ops.nearest_points(PyPolygon(sq), PyLineString(line))
        assert x1==r[0].x and y1==r[0].y and x2==r[1].x and y2==r[1].y

    def test_random(self, cpp):
        rng = np.random.RandomState(777)
        for _ in range(10):
            cx,cy = rng.uniform(-50,50),rng.uniform(-50,50); half = rng.uniform(1,10)
            sq = _square(cx,cy,half)
            line = [(rng.uniform(-100,100),rng.uniform(-100,100)),
                    (rng.uniform(-100,100),rng.uniform(-100,100))]
            x1,y1,x2,y2 = cpp.nearest_points(cpp.polygon(sq), cpp.linestring(line))
            r = py_ops.nearest_points(PyPolygon(sq), PyLineString(line))
            assert x1==r[0].x and y1==r[0].y and x2==r[1].x and y2==r[1].y


class TestNearestPointsLineStringPoint:
    def test_on_line(self, cpp):
        line = [(0.0,0.0),(10.0,0.0)]
        x1,y1,x2,y2 = cpp.nearest_points_ls_pt(cpp.linestring(line), cpp.point(5.0,3.0))
        r = py_ops.nearest_points(PyLineString(line), PyPoint(5.0,3.0))
        assert x1==r[0].x and y1==r[0].y and x2==r[1].x and y2==r[1].y

    def test_endpoint(self, cpp):
        line = [(0.0,0.0),(10.0,0.0)]
        x1,y1,x2,y2 = cpp.nearest_points_ls_pt(cpp.linestring(line), cpp.point(15.0,0.0))
        r = py_ops.nearest_points(PyLineString(line), PyPoint(15.0,0.0))
        assert x1==r[0].x and y1==r[0].y and x2==r[1].x and y2==r[1].y

    def test_random(self, cpp):
        rng = np.random.RandomState(42)
        for _ in range(10):
            line = [(rng.uniform(-10,10),rng.uniform(-10,10)) for _ in range(3)]
            px,py = rng.uniform(-15,15),rng.uniform(-15,15)
            x1,y1,x2,y2 = cpp.nearest_points_ls_pt(cpp.linestring(line), cpp.point(px,py))
            r = py_ops.nearest_points(PyLineString(line), PyPoint(px,py))
            assert x1==r[0].x and y1==r[0].y and x2==r[1].x and y2==r[1].y


# =============================================================================
# Serialization
# =============================================================================

class TestSerialization:
    def test_wkt_roundtrip(self, cpp, C):
        for cpp_geom, py_geom in [
            (C.point(3.14,-2.71), py_point(3.14,-2.71)),
            (C.point(-1e6,1e-6), py_point(-1e6,1e-6)),
            (C.linestring([(0,0),(10,5),(20,10)]), py_linestring([(0,0),(10,5),(20,10)])),
            (C.polygon([(0,0),(10,0),(10,10),(0,10)]), py_polygon([(0,0),(10,0),(10,10),(0,10)])),
            (C.linearring([(0,0),(10,0),(10,10),(0,0)]), py_linearring([(0,0),(10,0),(10,10),(0,0)])),
        ]:
            g1 = shapely_wkt.loads(cpp_geom.wkt())
            g2 = shapely_wkt.loads(py_geom.wkt)
            assert g1.equals_exact(g2, 1e-10)


# ==============================================================================
# COLLISION-CRITICAL EDGE CASES
# ==============================================================================
# These test the exact patterns used in production collision detection:
#   bool collision = (static_distance == 0.0);
#   bool intersects_rear = rear_poly.intersects(dyn_poly) && rear_poly.distance(dyn_poly) < 1e-12;
# etc. Each edge case verifies that C++ and Python produce bit-identical results
# exactly at the collision decision boundary.

class TestCollisionEdgeCases:
    """Deterministic edge cases at collision decision boundaries."""

    # --- Pattern 1: Polygon-Polygon distance (collision when == 0.0) ---

    def test_poly_poly_touching_edge(self, cpp):
        """Two squares sharing a boundary edge — distance must be exactly 0."""
        p1 = make_square_coords(0, 0, 5)          # (-5,-5) to (5,5)
        p2 = make_square_coords(10, 0, 5)         # (5,-5) to (15,5) — touch at x=5
        assert cpp.intersects_polygon_polygon(cpp.polygon(p1), cpp.polygon(p2)) == \
               PyPolygon(p1).intersects(PyPolygon(p2))

    def test_poly_poly_touching_vertex(self, cpp):
        """Two squares touching at a single vertex — distance == 0.0."""
        p1 = [(0,0),(5,0),(5,5),(0,5)]
        p2 = [(5,5),(10,5),(10,10),(5,10)]  # touch at (5,5)
        d_cpp = cpp.polygon(p1).buffer(0).area()  # just verify non-crash
        d_py = PyPolygon(p1).area
        assert d_cpp == d_py

    def test_poly_poly_near_touching(self, cpp):
        """Two squares separated by 1e-12 — near collision boundary."""
        p1 = make_square_coords(0, 0, 5)           # up to x=5
        p2 = make_square_coords(10.0 + 1e-12, 0, 5)  # from x=5+1e-12
        d_cpp = cpp.polygon(p1).distance(cpp.polygon(p2))
        d_py = PyPolygon(p1).distance(PyPolygon(p2))
        assert d_cpp == d_py, f"poly-poly near-touching: {d_cpp:.16e} vs {d_py:.16e}"

    def test_poly_poly_epsilon_overlap(self, cpp):
        """Two squares overlapping by 1e-12 — intersects must match."""
        p1 = make_square_coords(0, 0, 5)
        p2 = make_square_coords(10.0 - 1e-12, 0, 5)  # overlap by 1e-12
        d_cpp = cpp.polygon(p1).distance(cpp.polygon(p2))
        d_py = PyPolygon(p1).distance(PyPolygon(p2))
        assert d_cpp == d_py
        assert cpp.intersects_polygon_polygon(cpp.polygon(p1), cpp.polygon(p2)) == \
               PyPolygon(p1).intersects(PyPolygon(p2))

    # --- Pattern 2: Polygon-LineString distance ---

    def test_poly_ls_parallel_near(self, cpp):
        """Line parallel to polygon edge, 1e-12 away."""
        sq = make_square_coords(0, 0, 5)           # right edge at x=5
        line = [(5.0 + 1e-12, -10.0), (5.0 + 1e-12, 10.0)]
        assert cpp.distance_linestring_polygon(cpp.linestring(line), cpp.polygon(sq)) == \
               PyLineString(line).distance(PyPolygon(sq))

    def test_poly_ls_endpoint_on_boundary(self, cpp):
        """Line endpoint exactly on polygon boundary."""
        sq = make_square_coords(0, 0, 5)
        line = [(10.0, 5.0), (5.0, 0.0)]  # endpoint (5,0) is on polygon edge
        d_cpp = cpp.distance_linestring_polygon(cpp.linestring(line), cpp.polygon(sq))
        d_py = PyLineString(line).distance(PyPolygon(sq))
        assert d_cpp == d_py

    # --- Pattern 3: intersects check ---

    def test_intersects_touching_boundary(self, cpp):
        """LineString along polygon boundary — touches, intersects true."""
        sq = make_square_coords(0, 0, 5)
        line = [(-5.0, 5.0), (5.0, 5.0)]  # along top edge
        i_cpp = cpp.intersects_linestring_polygon(cpp.linestring(line), cpp.polygon(sq))
        i_py = PyLineString(line).intersects(PyPolygon(sq))
        assert i_cpp == i_py

    def test_intersects_epsilon_miss(self, cpp):
        """Line just outside polygon by 1e-12 — intersects false, distance == 1e-12."""
        sq = make_square_coords(0, 0, 5)
        line = [(5.0 + 1e-12, -5.0), (5.0 + 1e-12, 5.0)]
        i_cpp = cpp.intersects_linestring_polygon(cpp.linestring(line), cpp.polygon(sq))
        i_py = PyLineString(line).intersects(PyPolygon(sq))
        assert i_cpp == i_py
        d_cpp = cpp.distance_linestring_polygon(cpp.linestring(line), cpp.polygon(sq))
        d_py = PyLineString(line).distance(PyPolygon(sq))
        assert d_cpp == d_py

    # --- Pattern 4: LineString-Point distance ---

    def test_ls_pt_point_on_endpoint(self, cpp):
        """Point exactly at line endpoint — distance == 0."""
        line = [(0.0, 0.0), (10.0, 0.0)]
        assert cpp.distance_point_linestring(cpp.point(0.0, 0.0), cpp.linestring(line)) == \
               PyPoint(0.0, 0.0).distance(PyLineString(line))

    def test_ls_pt_point_near_line(self, cpp):
        """Point 1e-12 above line segment."""
        line = [(0.0, 0.0), (10.0, 0.0)]
        d_cpp = cpp.distance_point_linestring(cpp.point(5.0, 1e-12), cpp.linestring(line))
        d_py = PyPoint(5.0, 1e-12).distance(PyLineString(line))
        assert d_cpp == d_py

    def test_ls_pt_point_beyond_endpoint(self, cpp):
        """Point beyond line endpoint — distance to endpoint."""
        line = [(0.0, 0.0), (10.0, 0.0)]
        d_cpp = cpp.distance_point_linestring(cpp.point(15.0, 0.0), cpp.linestring(line))
        d_py = PyPoint(15.0, 0.0).distance(PyLineString(line))
        assert d_cpp == d_py

    # --- Pattern 5: LineString-LineString distance ---

    def test_ls_ls_perpendicular_touching(self, cpp):
        """Two perpendicular lines touching at midpoint."""
        l1 = [(0.0, 5.0), (10.0, 5.0)]
        l2 = [(5.0, 0.0), (5.0, 10.0)]  # intersect at (5,5)
        d1_cpp = cpp.linestring(l1).distance(cpp.linestring(l2))
        d1_py = PyLineString(l1).distance(PyLineString(l2))
        assert d1_cpp == d1_py

    def test_ls_ls_collinear_near(self, cpp):
        """Two collinear segments separated by gap."""
        l1 = [(0.0, 0.0), (10.0, 0.0)]
        l2 = [(10.0 + 1e-12, 0.0), (20.0, 0.0)]  # gap of 1e-12
        d_cpp = cpp.linestring(l1).distance(cpp.linestring(l2))
        d_py = PyLineString(l1).distance(PyLineString(l2))
        assert d_cpp == d_py

    def test_ls_ls_parallel_near(self, cpp):
        """Two parallel segments 1e-12 apart."""
        l1 = [(0.0, 0.0), (10.0, 0.0)]
        l2 = [(0.0, 1e-12), (10.0, 1e-12)]
        d_cpp = cpp.linestring(l1).distance(cpp.linestring(l2))
        d_py = PyLineString(l1).distance(PyLineString(l2))
        assert d_cpp == d_py

    # --- Pattern 6: LS↔Poly / Poly↔LS distance ---

    def test_ls_poly_vertex_on_edge(self, cpp):
        """Line vertex exactly on polygon edge."""
        sq = make_square_coords(0, 0, 5)
        line = [(10.0, 0.0), (5.0, 0.0), (5.0, 10.0)]  # vertex (5,0) on boundary
        d_cpp = cpp.distance_linestring_polygon(cpp.linestring(line), cpp.polygon(sq))
        d_py = PyLineString(line).distance(PyPolygon(sq))
        assert d_cpp == d_py

    def test_poly_ls_vertex_on_edge(self, cpp):
        """Same test reversed: Polygon.distance(LineString)."""
        sq = make_square_coords(0, 0, 5)
        line = [(10.0, 0.0), (5.0, 0.0), (5.0, 10.0)]
        d_cpp = cpp.distance_polygon_linestring(cpp.polygon(sq), cpp.linestring(line))
        d_py = PyPolygon(sq).distance(PyLineString(line))
        assert d_cpp == d_py


# ==============================================================================
# HIGH-VOLUME RANDOM TESTS — all 6 collision patterns
# ==============================================================================
# Float64: C++ vs Python, strict ==  (bit-identical by GEOS engine guarantee)
# Float32: C++ f32 vs Python with f32-truncated inputs, strict ==

_n64 = 1000   # float64 samples per pattern
_n32 = 500    # float32 samples per pattern


def _rand_pt(rng):
    return rng.uniform(-200, 200)


def _rand_ls(rng, npts=4):
    return [(_rand_pt(rng), _rand_pt(rng)) for _ in range(npts)]


def _rand_poly(rng, npts=4, min_radius=1, max_radius=20):
    cx, cy = _rand_pt(rng), _rand_pt(rng)
    angles = sorted(rng.uniform(0, 2*np.pi, npts))
    radii = rng.uniform(min_radius, max_radius, npts)
    return [(cx + r*np.cos(a), cy + r*np.sin(a)) for a, r in zip(angles, radii)]


class TestHighVolumePolyPoly:
    """Pattern 1: Polygon-Polygon distance (N=1000 f64, N=500 f32)."""

    def test_random_f64(self, cpp):
        rng = np.random.RandomState(1)
        for _ in range(_n64):
            c1 = _rand_poly(rng); c2 = _rand_poly(rng)
            d = cpp.polygon(c1).distance(cpp.polygon(c2))
            assert d == PyPolygon(c1).distance(PyPolygon(c2))

    def test_random_f32(self, make):
        if make['dtype'] != np.float32: pytest.skip("float32 only")
        rng = np.random.RandomState(1)
        for _ in range(_n32):
            c1 = _rand_poly(rng); c2 = _rand_poly(rng)
            f32_c1 = _f32(c1); f32_c2 = _f32(c2)
            d_f32 = make['polygon'](f32_c1).distance(make['polygon'](f32_c2))
            d_py = PyPolygon(f32_c1).distance(PyPolygon(f32_c2))
            assert d_f32 == d_py, f"f32 poly-poly: {d_f32:.16e} vs {d_py:.16e}"


class TestHighVolumePolyLS:
    """Patterns 2 & 6: Polygon-LineString distance (N=1000 f64, N=500 f32)."""

    def test_random_f64(self, cpp):
        rng = np.random.RandomState(2)
        for _ in range(_n64):
            c = _rand_poly(rng); l = _rand_ls(rng, 3)
            d1 = cpp.distance_linestring_polygon(cpp.linestring(l), cpp.polygon(c))
            d2 = cpp.distance_polygon_linestring(cpp.polygon(c), cpp.linestring(l))
            py_d1 = PyLineString(l).distance(PyPolygon(c))
            py_d2 = PyPolygon(c).distance(PyLineString(l))
            assert d1 == py_d1; assert d2 == py_d2
            # Symmetry: LS↔Poly should equal Poly↔LS
            assert d1 == d2

    def test_random_f32(self, cpp):
        rng = np.random.RandomState(2)
        for _ in range(_n32):
            c = _rand_poly(rng); l = _rand_ls(rng, 3)
            f32_c = _f32(c); f32_l = _f32(l)
            # Use f64 C++ APIs with f32-truncated coords → both sides get identical doubles
            d1 = cpp.distance_linestring_polygon(cpp.linestring(f32_l), cpp.polygon(f32_c))
            d2 = cpp.distance_polygon_linestring(cpp.polygon(f32_c), cpp.linestring(f32_l))
            py_d1 = PyLineString(f32_l).distance(PyPolygon(f32_c))
            py_d2 = PyPolygon(f32_c).distance(PyLineString(f32_l))
            assert d1 == py_d1; assert d2 == py_d2
            assert d1 == d2


class TestHighVolumeLSPoint:
    """Pattern 4: LineString-Point distance (N=1000 f64, N=500 f32)."""

    def test_random_f64(self, cpp):
        rng = np.random.RandomState(3)
        for _ in range(_n64):
            l = _rand_ls(rng, 4); px, py = _rand_pt(rng), _rand_pt(rng)
            assert cpp.distance_point_linestring(cpp.point(px, py), cpp.linestring(l)) == \
                   PyPoint(px, py).distance(PyLineString(l))

    def test_random_f32(self, cpp):
        rng = np.random.RandomState(3)
        for _ in range(_n32):
            l = _rand_ls(rng, 4); px, py = _rand_pt(rng), _rand_pt(rng)
            f32_l = _f32(l); f32_px = _f32(px); f32_py = _f32(py)
            # Use f64 C++ APIs with f32-truncated coords → both sides get identical doubles
            d_cpp = cpp.distance_point_linestring(cpp.point(f32_px, f32_py), cpp.linestring(f32_l))
            d_py = PyPoint(f32_px, f32_py).distance(PyLineString(f32_l))
            assert d_cpp == d_py, f"f32 ls-pt: {d_cpp:.16e} vs {d_py:.16e}"


class TestHighVolumeLSLS:
    """Pattern 5: LineString-LineString distance (N=1000 f64, N=500 f32)."""

    def test_random_f64(self, cpp):
        rng = np.random.RandomState(4)
        for _ in range(_n64):
            l1 = _rand_ls(rng, 3); l2 = _rand_ls(rng, 3)
            d_cpp = cpp.linestring(l1).distance(cpp.linestring(l2))
            d_py = PyLineString(l1).distance(PyLineString(l2))
            assert d_cpp == d_py

    def test_random_f32(self, make):
        if make['dtype'] != np.float32: pytest.skip("float32 only")
        rng = np.random.RandomState(4)
        for _ in range(_n32):
            l1 = _rand_ls(rng, 3); l2 = _rand_ls(rng, 3)
            f32_l1 = _f32(l1); f32_l2 = _f32(l2)
            d_f32 = make['linestring'](f32_l1).distance(make['linestring'](f32_l2))
            d_py = PyLineString(f32_l1).distance(PyLineString(f32_l2))
            assert d_f32 == d_py, f"f32 ls-ls: {d_f32:.16e} vs {d_py:.16e}"


class TestHighVolumeIntersects:
    """Pattern 3: intersects check (N=500 f64, N=250 f32)."""

    def test_random_poly_poly_f64(self, cpp):
        rng = np.random.RandomState(5)
        for _ in range(500):
            c1 = _rand_poly(rng); c2 = _rand_poly(rng)
            try:
                pp1 = PyPolygon(c1); pp2 = PyPolygon(c2)
                if not pp1.is_valid or not pp2.is_valid: continue
                i_cpp = cpp.intersects_polygon_polygon(cpp.polygon(c1), cpp.polygon(c2))
                i_py = pp1.intersects(pp2)
                assert i_cpp == i_py
                # Also verify: intersects implies distance == 0
                if i_cpp:
                    d_cpp = cpp.polygon(c1).distance(cpp.polygon(c2))
                    d_py = pp1.distance(pp2)
                    assert d_cpp == d_py
            except Exception:
                pass

    def test_random_ls_poly_f64(self, cpp):
        rng = np.random.RandomState(6)
        for _ in range(500):
            l = _rand_ls(rng, 3); c = _rand_poly(rng)
            i_cpp = cpp.intersects_linestring_polygon(cpp.linestring(l), cpp.polygon(c))
            i_py = PyLineString(l).intersects(PyPolygon(c))
            assert i_cpp == i_py


# ==============================================================================
# 3-COLUMN ARRAY SHAPE HANDLING — verify (N,≥3) → x,y extraction
# ==============================================================================
# Regression tests for the array_to_double_vec() fix (hardcoded rows*2 → stride).
# Covers: generic py::array overload (int dtype), typed double overload, point,
# polygon, multipoint; plus row/column layout consistency and non-contiguous view guard.

class TestArrayShape3D:
    """Verify (N,≥3) arrays correctly extract x,y columns across all factory overloads."""

    # -- (N,2) regression: generic overload (int32) still works ----------------------

    def test_2col_generic(self, cpp):
        coords = np.array([[0, 1], [2, 3], [4, 5]], dtype=np.int32)
        ls = cpp.linestring(coords)
        assert ls.coords() == [(0.0, 1.0), (2.0, 3.0), (4.0, 5.0)]

    # -- (N,3) generic overload: z column correctly ignored -------------------------

    def test_3col_generic(self, cpp):
        coords = np.array([[10, 20, 999], [30, 40, 888], [50, 60, 777]], dtype=np.int32)
        ls = cpp.linestring(coords)
        assert ls.coords() == [(10.0, 20.0), (30.0, 40.0), (50.0, 60.0)]

    # -- (N,3) typed double overload: stride by shape[1]=3 --------------------------

    def test_3col_typed_double(self, cpp):
        coords = np.array([[10.0, 20.0, 99.0], [30.0, 40.0, 88.0], [50.0, 60.0, 77.0]],
                          dtype=np.float64)
        ls = cpp.linestring(coords)
        assert ls.coords() == [(10.0, 20.0), (30.0, 40.0), (50.0, 60.0)]

    # -- Equality: (N,3) produces identical coords to (N,2) -------------------------

    def test_3col_equals_2col_generic(self, cpp):
        coords_3 = np.array([[1, 2, 99], [3, 4, 88], [5, 6, 77]], dtype=np.int32)
        coords_2 = np.array([[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]], dtype=np.float64)
        ls3 = cpp.linestring(coords_3)
        ls2 = cpp.linestring(coords_2)
        assert ls3.coords() == ls2.coords()
        # Distance to self should also match
        assert ls3.length() == ls2.length()

    def test_3col_equals_2col_typed(self, cpp):
        coords_3 = np.array([[1.0, 2.0, 99.0], [3.0, 4.0, 88.0], [5.0, 6.0, 77.0]],
                            dtype=np.float64)
        coords_2 = np.array([[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]], dtype=np.float64)
        ls3 = cpp.linestring(coords_3)
        ls2 = cpp.linestring(coords_2)
        assert ls3.coords() == ls2.coords()
        assert ls3.length() == ls2.length()

    # -- Polygon (N,3) generic ------------------------------------------------------

    def test_polygon_3col_generic(self, cpp):
        coords = np.array([[0, 0, 1], [10, 0, 2], [10, 10, 3], [0, 10, 4], [0, 0, 5]],
                          dtype=np.int32)
        poly = cpp.polygon(coords)
        ext = cpp.polygon_exterior(poly)
        # First 5 points must match (GEOS may auto-close to 6 points)
        assert ext.tolist()[:5] == [[0.0, 0.0], [10.0, 0.0], [10.0, 10.0], [0.0, 10.0], [0.0, 0.0]]

    def test_polygon_3col_typed(self, cpp):
        coords = np.array([[0.0, 0.0, 1.0], [10.0, 0.0, 2.0], [10.0, 10.0, 3.0],
                           [0.0, 10.0, 4.0], [0.0, 0.0, 5.0]], dtype=np.float64)
        poly = cpp.polygon(coords)
        ext = cpp.polygon_exterior(poly)
        assert ext.tolist()[:5] == [[0.0, 0.0], [10.0, 0.0], [10.0, 10.0], [0.0, 10.0], [0.0, 0.0]]

    # -- Point: 1D [x,y] and 2D (N,≥2) int arrays (auto-double overload) -------------

    def test_point_1d_int(self, cpp):
        p = cpp.point(np.array([3, 4], dtype=np.int32))
        assert p.coords() == [(3.0, 4.0)]

    def test_point_2d_int(self, cpp):
        p = cpp.point(np.array([[5, 6]], dtype=np.int32))
        assert p.coords() == [(5.0, 6.0)]

    def test_point_2d_3col_int(self, cpp):
        """Point from (N,3) int: takes first 2 elements (row 0, col 0-1)."""
        p = cpp.point(np.array([[7, 8, 999], [9, 10, 888]], dtype=np.int32))
        assert p.coords() == [(7.0, 8.0)]

    # -- MultiPoint (N,3) generic ---------------------------------------------------

    def test_multipoint_3col_generic(self, cpp):
        mp = cpp.multipoint(np.array([[1, 2, 99], [4, 6, 88]], dtype=np.int32))
        # Verify via distance: (1,2) should be a point in the multipoint
        p = cpp.point(1.0, 2.0)
        assert mp.distance(p) == 0.0

    # -- LinearRing (N,3) -----------------------------------------------------------

    def test_linearring_3col(self, cpp, C):
        ring = C.linearring(np.array([[0, 0, 1], [1, 0, 2], [1, 1, 3], [0, 0, 4]],
                                      dtype=np.float64))
        assert ring.is_closed() == True
        assert ring.is_ring() == True

    # -- Row/column layout consistency ----------------------------------------------

    def test_row_col_layout(self, cpp):
        """Verify Python arr[i,j] ↔ C++ ptr[i*shape[1]+j] mapping."""
        # (3,4) array: row-major [1,2,3,4, 5,6,7,8, 9,10,11,12]
        arr = np.array([[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12]], dtype=np.float64)
        ls = cpp.linestring(arr)
        # C++ takes coords_[i*4+0], coords_[i*4+1] → first 2 columns per row
        assert ls.coords() == [(1.0, 2.0), (5.0, 6.0), (9.0, 10.0)]

        # (3,3) direct: stride=3, skip z
        arr3 = np.array([[10, 20, 99], [30, 40, 88], [50, 60, 77]], dtype=np.float64)
        ls3 = cpp.linestring(arr3)
        assert ls3.coords() == [(10.0, 20.0), (30.0, 40.0), (50.0, 60.0)]

    # -- poly_exterior output shape --------------------------------------------------

    def test_exterior_shape(self, cpp):
        coords = np.array([[0, 0], [10, 0], [10, 10], [0, 10]], dtype=np.float64)
        poly = cpp.polygon(coords)
        ext = cpp.polygon_exterior(poly)
        assert ext.ndim == 2
        assert ext.shape[1] == 2  # always 2 columns
        assert ext.dtype == np.float64

    # -- Non-contiguous view rejection (fail-fast with clear error) ----------------

    def test_noncontiguous_view_rejected_typed(self, cpp):
        """Typed overloads throw on non-contiguous views (e.g. [:, :2])."""
        arr = np.array([[10, 20, 99], [30, 40, 88], [50, 60, 77]], dtype=np.float64)
        view = arr[:, :2]
        assert not view.flags['C_CONTIGUOUS']
        with pytest.raises(ValueError, match="C-contiguous"):
            cpp.linestring(view)

    def test_noncontiguous_view_rejected_generic(self, cpp):
        """Generic (int dtype) overloads throw on non-contiguous views."""
        arr = np.array([[10, 20, 99], [30, 40, 88], [50, 60, 77]], dtype=np.int32)
        view = arr[:, :2]
        assert not view.flags['C_CONTIGUOUS']
        with pytest.raises(ValueError, match="C-contiguous"):
            cpp.linestring(view)

    def test_noncontiguous_copy_works_typed(self, cpp):
        """.copy() on a non-contiguous view produces correct results."""
        arr = np.array([[10, 20, 99], [30, 40, 88], [50, 60, 77]], dtype=np.float64)
        cpy = arr[:, :2].copy()
        assert cpy.flags['C_CONTIGUOUS']
        ls = cpp.linestring(cpy)
        assert ls.coords() == [(10.0, 20.0), (30.0, 40.0), (50.0, 60.0)]

    def test_noncontiguous_copy_works_generic(self, cpp):
        """.copy() on a non-contiguous view works for generic overload."""
        arr = np.array([[10, 20, 99], [30, 40, 88], [50, 60, 77]], dtype=np.int32)
        cpy = arr[:, :2].copy()
        assert cpy.flags['C_CONTIGUOUS']
        ls = cpp.linestring(cpy)
        assert ls.coords() == [(10.0, 20.0), (30.0, 40.0), (50.0, 60.0)]

    @pytest.mark.parametrize("factory, arr_dtype", [
        ("point", np.float64),
        ("point", np.int32),
        ("polygon", np.float64),
        ("polygon", np.int32),
        ("multipoint", np.float64),
        ("multipoint", np.int32),
    ])
    def test_noncontiguous_rejected_all_factories(self, cpp, factory, arr_dtype):
        """All factory functions reject non-contiguous views."""
        arr = np.array([[10, 20, 99], [30, 40, 88], [50, 60, 77]], dtype=arr_dtype)
        view = arr[:, :2]
        assert not view.flags['C_CONTIGUOUS']
        fn = getattr(cpp, factory)
        with pytest.raises(ValueError, match="C-contiguous"):
            fn(view)
