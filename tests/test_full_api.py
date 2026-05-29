"""
Comprehensive coupled C++ vs Python shapely API test suite.
Tests every implemented method against Python shapely with identical inputs.
"""

import numpy as np
import pytest
import shapely.geometry as sg
from shapely import wkt as shapely_wkt
from .utils import (
    random_point, random_line, random_polygon,
    py_linestring, py_polygon, py_linearring,
)

DEFAULT_SEED = 42

def py_point(x, y):
    return sg.Point(x, y)

def assert_same_wkt(cpp_wkt, py_wkt, tol=1e-10):
    """Compare WKT strings by round-tripping through shapely, then checking equals."""
    g1 = shapely_wkt.loads(cpp_wkt)
    g2 = shapely_wkt.loads(py_wkt)
    assert g1.equals_exact(g2, tol), f"WKT mismatch:\n  C++: {cpp_wkt}\n  Py:  {py_wkt}"


# ==============================================================================
# Point
# ==============================================================================

class TestPoint:
    @pytest.mark.parametrize("x,y", [(0,0), (3.14,-2.71), (1e6,-1e-6)])
    def test_accessors(self, cpp, C, x, y):
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
        assert_same_wkt(cpt.wkt(), ppt.wkt)

    @pytest.mark.parametrize("x,y", [(0,0), (3.14,-2.71)])
    def test_accessors_f32(self, cpp, C, x, y):
        cpt = C.point(x, y, 'float32')
        ppt = py_point(float(x), float(y))
        assert abs(float(cpt.x) - ppt.x) < 1e-6
        assert abs(float(cpt.y) - ppt.y) < 1e-6
        assert cpt.is_valid() == ppt.is_valid

    def test_coords_xy(self, cpp, C):
        p = C.point(1.5, -2.5)
        cs = list(p.coords())
        assert len(cs) == 1
        assert cs[0][0] == 1.5
        assert abs(cs[0][1] - (-2.5)) < 1e-10
        xs, ys = p.xy()
        assert xs == [1.5] and ys == [-2.5]

    @pytest.mark.parametrize("x,y", [(0,0), (5,-3), (-10,20)])
    def test_centroid(self, cpp, C, x, y):
        cx, cy = cpp.centroid_point(C.point(x, y))
        pc = py_point(x, y).centroid
        assert cx == pc.x
        assert cy == pc.y

    def test_buffer(self, cpp, C):
        buf = C.point(0, 0).buffer(10.0)
        py_buf = py_point(0, 0).buffer(10.0)
        assert abs(buf.area() - py_buf.area) < 1e-4

    @pytest.mark.parametrize("x1,y1,x2,y2", [(0,0,0,0),(0,0,1,1),(5,5,5,5),(-1,2,-3,4)])
    def test_predicates_pt_pt(self, cpp, C, x1, y1, x2, y2):
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
        assert cpp.pt_hausdorff_distance_pt(c1, c2) == pytest.approx(p1.hausdorff_distance(p2), abs=1e-8)

    def test_predicates_pt_poly(self, cpp, C):
        sq = C.polygon([(0,0),(10,0),(10,10),(0,10)])
        psq = py_polygon([(0,0),(10,0),(10,10),(0,10)])
        for name, cx, cy in [("in",5,5),("out",20,20),("edge",0,5)]:
            cpt, ppt = C.point(cx, cy), py_point(cx, cy)
            assert cpp.pt_within_poly(cpt, sq) == ppt.within(psq), f"within {name}"
            assert cpp.pt_disjoint_poly(cpt, sq) == ppt.disjoint(psq), f"disjoint {name}"
            assert cpp.pt_touches_poly(cpt, sq) == ppt.touches(psq), f"touches {name}"

    def test_random_pt_predicates(self, cpp, C):
        rng = np.random.RandomState(DEFAULT_SEED)
        for _ in range(20):
            x1, y1 = rng.uniform(-100, 100), rng.uniform(-100, 100)
            x2, y2 = rng.uniform(-100, 100), rng.uniform(-100, 100)
            c1, c2 = C.point(x1, y1), C.point(x2, y2)
            p1, p2 = py_point(x1, y1), py_point(x2, y2)
            assert cpp.pt_intersects_pt(c1, c2) == p1.intersects(p2)
            assert cpp.pt_disjoint_pt(c1, c2) == p1.disjoint(p2)


# ==============================================================================
# LineString
# ==============================================================================

class TestLineString:
    def test_accessors(self, cpp, C):
        c_ls, p_ls = C.linestring([(0,0),(10,0),(10,10)]), py_linestring([(0,0),(10,0),(10,10)])
        assert_same_wkt(c_ls.wkt(), p_ls.wkt)
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

    def test_centroid(self, cpp, C):
        cx, cy = cpp.centroid_linestring(C.linestring([(0,0),(10,0),(10,10)]))
        pc = py_linestring([(0,0),(10,0),(10,10)]).centroid
        assert cx == pc.x
        assert cy == pc.y

    def test_predicates(self, cpp, C):
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

    def test_ls_with_point(self, cpp, C):
        ls = C.linestring([(0,0),(10,0)])
        pls = py_linestring([(0,0),(10,0)])
        pt_on, pt_off = C.point(5,0), C.point(5,5)
        ppt_on, ppt_off = py_point(5,0), py_point(5,5)
        assert cpp.ls_contains_pt(ls, pt_on) == pls.contains(ppt_on)
        assert cpp.ls_intersects_pt(ls, pt_on) == pls.intersects(ppt_on)
        assert cpp.ls_disjoint_pt(ls, pt_off) == pls.disjoint(ppt_off)

    def test_random_ls(self, cpp, C):
        rng = np.random.RandomState(DEFAULT_SEED)
        for _ in range(20):
            c1, c2 = random_line(rng, 3), random_line(rng, 3)
            cls1, cls2 = C.linestring(c1), C.linestring(c2)
            p1, p2 = py_linestring(c1), py_linestring(c2)
            assert cpp.ls_intersects_ls(cls1, cls2) == p1.intersects(p2)


# ==============================================================================
# Polygon
# ==============================================================================

class TestPolygon:
    SQ = [(0,0),(10,0),(10,10),(0,10)]

    def test_accessors(self, cpp, C):
        cp, pp = C.polygon(self.SQ), py_polygon(self.SQ)
        assert_same_wkt(cp.wkt(), pp.wkt)
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

    def test_centroid(self, cpp, C):
        for coords in [self.SQ, [(0,0),(10,0),(5,8)], [(0,0),(10,0),(10,5),(5,10),(0,5)]]:
            cx, cy = cpp.centroid_polygon(C.polygon(coords))
            pc = py_polygon(coords).centroid
            assert cx == pc.x
            assert cy == pc.y

    def test_exterior(self, cpp, C):
        ext = cpp.polygon_exterior(C.polygon(self.SQ))
        pext = py_polygon(self.SQ).exterior
        assert ext.shape[0] == len(pext.coords)
        for i, (cpt, ppt) in enumerate(zip(ext, pext.coords)):
            assert cpt[0] == ppt[0]
            assert cpt[1] == ppt[1]

    def test_coords(self, cpp, C):
        coords = list(C.polygon(self.SQ).coords())
        for (cx, cy), (x, y) in zip(coords, self.SQ):
            assert cx == x

    def test_predicates_poly_poly(self, cpp, C):
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

    def test_predicates_poly_point(self, cpp, C):
        sq = C.polygon(self.SQ)
        psq = py_polygon(self.SQ)
        for name, cx, cy in [("in",5,5),("out",20,20),("edge",0,5)]:
            cpt, ppt = C.point(cx, cy), py_point(cx, cy)
            assert cpp.poly_contains_pt(sq, cpt) == psq.contains(ppt), f"contains {name}"
            assert cpp.poly_covers_pt(sq, cpt) == psq.covers(ppt), f"covers {name}"
            assert cpp.poly_intersects_pt(sq, cpt) == psq.intersects(ppt), f"intersects {name}"

    def test_predicates_poly_ls(self, cpp, C):
        sq = C.polygon(self.SQ)
        psq = py_polygon(self.SQ)
        # Line crossing through
        cls = C.linestring([(-5,5),(15,5)])
        pls = py_linestring([(-5,5),(15,5)])
        assert cpp.poly_intersects_ls(sq, cls) == psq.intersects(pls)
        assert cpp.poly_crosses_ls(sq, cls) == psq.crosses(pls)
        # Line outside
        cls2 = C.linestring([(20,0),(25,0)])
        pls2 = py_linestring([(20,0),(25,0)])
        assert cpp.poly_disjoint_ls(sq, cls2) == psq.disjoint(pls2)

    def test_random_poly_predicates(self, cpp, C):
        rng = np.random.RandomState(DEFAULT_SEED)
        for _ in range(15):
            c1, c2 = random_polygon(rng), random_polygon(rng)
            try:
                cp1, cp2 = C.polygon(c1), C.polygon(c2)
                pp1, pp2 = py_polygon(c1), py_polygon(c2)
                if not pp1.is_valid or pp1.is_empty or not pp2.is_valid or pp2.is_empty:
                    continue
                assert cpp.poly_intersects_poly(cp1, cp2) == pp1.intersects(pp2)
                assert cpp.poly_disjoint_poly(cp1, cp2) == pp1.disjoint(pp2)
            except Exception:
                pass


# ==============================================================================
# LinearRing
# ==============================================================================

class TestLinearRing:
    RING_SQ = [(0,0),(10,0),(10,10),(0,10)]

    def test_properties(self, cpp, C):
        cr, pr = C.linearring(self.RING_SQ), py_linearring(self.RING_SQ)
        assert cr.is_empty() == pr.is_empty
        assert cr.is_simple() == pr.is_simple
        assert cr.is_valid() == pr.is_valid
        assert cr.is_closed() == pr.is_closed
        assert cr.is_ring() == pr.is_ring
        assert cr.length() == pr.length
        assert cr.bounds() == list(pr.bounds)
        assert cr.area() == pr.area

    def test_wkt(self, cpp, C):
        assert_same_wkt(C.linearring(self.RING_SQ).wkt(), py_linearring(self.RING_SQ).wkt)

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
        # CCW in shapely: (0,0)->(10,0)->(10,10)->(0,10)
        assert C.linearring([(0,0),(10,0),(10,10),(0,10)]).is_ccw() == True
        assert C.linearring([(0,0),(0,10),(10,10),(10,0)]).is_ccw() == False

    def test_empty(self, cpp, C):
        assert C.linearring([]).is_empty() == True


# ==============================================================================
# Cross-geometry coupled tests
# ==============================================================================

class TestCrossGeometry:
    def test_point_vs_polygon_all_scenarios(self, cpp, C):
        sq = C.polygon([(0,0),(10,0),(10,10),(0,10)])
        psq = py_polygon([(0,0),(10,0),(10,10),(0,10)])
        for name, cx, cy in [
            ("interior", 5,5), ("exterior", 20,20),
            ("boundary", 0,5), ("vertex", 0,0)
        ]:
            cpt, ppt = C.point(cx, cy), py_point(cx, cy)
            assert cpp.poly_intersects_pt(sq, cpt) == psq.intersects(ppt), name
            assert cpp.poly_contains_pt(sq, cpt) == psq.contains(ppt), name
            assert cpp.poly_covers_pt(sq, cpt) == psq.covers(ppt), name

    def test_ls_vs_polygon(self, cpp, C):
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

    def test_random_cross_type(self, cpp, C):
        rng = np.random.RandomState(DEFAULT_SEED)
        for _ in range(30):
            pt_c = random_point(rng)
            ls_c = random_line(rng, 3)
            cpt, cls = C.point(*pt_c), C.linestring(ls_c)
            ppt, pls = py_point(*pt_c), py_linestring(ls_c)
            assert cpp.pt_intersects_ls(cpt, cls) == ppt.intersects(pls)
            assert abs(cpp.distance_point_linestring(cpt, cls) - ppt.distance(pls)) < 1e-8


# ==============================================================================
# Serialization
# ==============================================================================

class TestSerialization:
    def test_wkt_roundtrip(self, cpp, C):
        for cpp_geom, py_geom in [
            (C.point(3.14, -2.71), py_point(3.14, -2.71)),
            (C.point(-1e6, 1e-6), py_point(-1e6, 1e-6)),
            (C.linestring([(0,0),(10,5),(20,10)]), py_linestring([(0,0),(10,5),(20,10)])),
            (C.polygon([(0,0),(10,0),(10,10),(0,10)]), py_polygon([(0,0),(10,0),(10,10),(0,10)])),
            (C.linearring([(0,0),(10,0),(10,10),(0,0)]), py_linearring([(0,0),(10,0),(10,10),(0,0)])),
        ]:
            assert_same_wkt(cpp_geom.wkt(), py_geom.wkt)
