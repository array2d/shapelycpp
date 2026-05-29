// shapelycpp — comprehensive native C++ usage example
// =====================================================
// Covers all geometry types, all API categories, and real-world
// collision detection patterns.  Zero pybind11 dependency.

#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>
#include "shapely/geometry/point.h"
#include "shapely/geometry/linestring.h"
#include "shapely/geometry/polygon.h"
#include "shapely/ops/nearest_points.h"

using namespace shapely::geometry;
using namespace shapely::ops;

void section(const char* title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

// ===========================================================================
// 1. Point: construction, accessors, distance, predicates, buffer
// ===========================================================================
void demo_point() {
    section("1. Point");

    Point<double> p1(0.0, 0.0);
    Point<double> p2(3.0, 4.0);
    Point<double> p3(1.0, 1.0);

    // -- accessors --
    std::cout << "p1.x = " << p1.x << ", p1.y = " << p1.y << std::endl;
    std::cout << "p1.wkt() = " << p1.wkt() << std::endl;
    std::cout << "p1.type() = " << p1.type() << std::endl;
    std::cout << "p1.is_valid() = " << p1.is_valid() << std::endl;
    std::cout << "p1.area() = " << p1.area() << " (always 0)" << std::endl;
    std::cout << "p1.length() = " << p1.length() << " (always 0)" << std::endl;

    auto b = p1.bounds();
    std::cout << "p1.bounds() = [" << b[0] << ", " << b[1] << ", " << b[2] << ", " << b[3] << "]" << std::endl;

    // -- coords / xy --
    auto coords = p1.coords();
    std::cout << "p1.coords() = [(" << std::get<0>(coords[0]) << ", " << std::get<1>(coords[0]) << ")]" << std::endl;

    auto [xs, ys] = p1.xy();
    std::cout << "p1.xy() → xs[0]=" << xs[0] << " ys[0]=" << ys[0] << std::endl;

    // -- distance --
    double d = p1.distance(p2);
    std::cout << "p1.distance(p2) = " << d << " (expected 5.0)" << std::endl;
    assert(std::abs(d - 5.0) < 1e-10);

    // -- predicates (same-type) --
    std::cout << "p1.disjoint(p2)  = " << p1.disjoint(p2) << " (expected 1)" << std::endl;
    std::cout << "p1.equals(p1)    = " << p1.equals(p1)  << " (expected 1)" << std::endl;
    std::cout << "p1.touches(p2)   = " << p1.touches(p2) << " (expected 0)" << std::endl;
    std::cout << "p1.within(p2)    = " << p1.within(p2)  << " (expected 0)" << std::endl;
    std::cout << "p1.contains(p1)  = " << p1.contains(p1) << " (expected 1)" << std::endl;
    std::cout << "p1.intersects(p2) = " << p1.intersects(p2) << " (expected 0)" << std::endl;

    // -- relate (DE-9IM) --
    std::cout << "p1.relate(p2) = \"" << p1.relate(p2) << "\" (expected FF0FFF0F2)" << std::endl;

    // -- buffer --
    auto buf = p1.buffer(5.0);
    std::cout << "p1.buffer(5.0).area() = " << buf.area() << " (expected ~78.54)" << std::endl;

    // -- centroid --
    auto cent = p1.centroid();
    std::cout << "p1.centroid() = (" << cent.x << ", " << cent.y << ")" << std::endl;

    // -- float32 Point --
    Point<float> fp1(1.5f, -2.5f);
    std::cout << "\n[float32] fp1 = (" << fp1.x << ", " << fp1.y << ")" << std::endl;
    std::cout << "fp1.is_valid() = " << fp1.is_valid() << std::endl;
    double fd = fp1.distance(Point<float>(4.5f, 1.5f));
    std::cout << "fp1.distance(Point<float>(4.5, 1.5)) = " << fd << " (expected 5.0)" << std::endl;
}

// ===========================================================================
// 2. LineString: construction, accessors, distance, predicates
// ===========================================================================
void demo_linestring() {
    section("2. LineString");

    double coords1[] = {0,0, 10,0, 10,10};
    LineString<double> ls1(coords1, 3);
    double coords2[] = {0,10, 10,10};
    LineString<double> ls2(coords2, 2);
    double coords3[] = {0,5, 10,5};
    LineString<double> ls3(coords3, 2);

    // -- accessors --
    std::cout << "ls1.wkt() = " << ls1.wkt() << std::endl;
    std::cout << "ls1.length() = " << ls1.length() << " (expected ~14.14)" << std::endl;
    std::cout << "ls1.is_closed() = " << ls1.is_closed() << std::endl;
    std::cout << "ls1.is_ring() = " << ls1.is_ring() << std::endl;
    std::cout << "ls1.is_empty() = " << ls1.is_empty() << std::endl;
    std::cout << "ls1.is_simple() = " << ls1.is_simple() << std::endl;

    auto coords = ls1.coords();
    std::cout << "ls1.coords() → " << coords.size() << " points" << std::endl;

    // -- same-type distance --
    std::cout << "\nls2.distance(ls3) = " << ls2.distance(ls3)
              << " (parallel, 5 apart, expected 5.0)" << std::endl;
    assert(std::abs(ls2.distance(ls3) - 5.0) < 1e-10);

    // -- predicates --
    std::cout << "ls2.intersects(ls3) = " << ls2.intersects(ls3) << " (expected 0 — parallel)" << std::endl;

    double crossing[] = {5,0, 5,10};
    LineString<double> ls_cross(crossing, 2);
    // (actually let me just use ls1 which is L-shaped)
    double vert[] = {5,-5, 5,15};
    LineString<double> ls_vert(vert, 2);
    std::cout << "ls1.crosses(ls_vert) = " << ls1.crosses(ls_vert) << " (expected 1)" << std::endl;
    std::cout << "ls1.intersects(ls_vert) = " << ls1.intersects(ls_vert) << " (expected 1)" << std::endl;

    // -- buffer --
    auto buf = ls2.buffer(2.0);
    std::cout << "ls2.buffer(2.0).area() = " << buf.area() << std::endl;
}

// ===========================================================================
// 3. Polygon: construction, area, predicates, distance, intersection
// ===========================================================================
void demo_polygon() {
    section("3. Polygon");

    double sq[] = {0,0, 10,0, 10,10, 0,10};        // 10×10 square at origin
    double sq2[] = {5,0, 15,0, 15,10, 5,10};        // 10×10 square, half-overlap
    double sq_far[] = {30,0, 40,0, 40,10, 30,10};   // 10×10 far away

    Polygon<double> poly1(sq, 4);
    Polygon<double> poly2(sq2, 4);
    Polygon<double> poly_far(sq_far, 4);
    Polygon<double> empty;  // default = empty

    // -- accessors --
    std::cout << "poly1.area() = " << poly1.area() << " (expected 100.0)" << std::endl;
    std::cout << "poly1.length() = " << poly1.length() << " (expected 40.0)" << std::endl;
    std::cout << "poly1.is_valid() = " << poly1.is_valid() << std::endl;
    std::cout << "poly1.is_empty() = " << poly1.is_empty() << std::endl;
    std::cout << "empty_poly.is_empty() = " << empty.is_empty() << " (expected 1)" << std::endl;
    std::cout << "poly1.wkt() = " << poly1.wkt() << std::endl;

    // -- exterior --
    auto ext = poly1.exterior();
    std::cout << "poly1.exterior().length() = " << ext.length() << " (expected 40.0)" << std::endl;
    std::cout << "poly1.exterior().is_closed() = " << ext.is_closed() << std::endl;

    // -- same-type distance --
    std::cout << "\npoly1.distance(poly2) = " << poly1.distance(poly2) << " (overlapping, expected 0.0)" << std::endl;
    std::cout << "poly1.distance(poly_far) = " << poly1.distance(poly_far) << " (separated: 20.0)" << std::endl;

    // -- predicates --
    std::cout << "poly1.overlaps(poly2) = " << poly1.overlaps(poly2) << " (expected 1)" << std::endl;
    std::cout << "poly1.intersects(poly2) = " << poly1.intersects(poly2) << " (expected 1)" << std::endl;
    std::cout << "poly1.disjoint(poly_far) = " << poly1.disjoint(poly_far) << " (expected 1)" << std::endl;
    std::cout << "poly1.touches(poly_far) = " << poly1.touches(poly_far) << " (expected 0)" << std::endl;
    std::cout << "poly1.contains(poly2) = " << poly1.contains(poly2) << " (expected 0 — partial)" << std::endl;

    // -- intersection --
    auto inter = poly1.intersection(poly2);
    std::cout << "\npoly1.intersection(poly2).area() = " << inter.area() << " (expected 50.0)" << std::endl;

    // -- buffer --
    auto buf = poly1.buffer(2.0);
    std::cout << "poly1.buffer(2.0).area() = " << buf.area()
              << " (base 100 + ~80 border, expected ~196)" << std::endl;

    // -- centroid --
    auto cent = poly1.centroid();
    std::cout << "poly1.centroid() = (" << cent.x << ", " << cent.y << ") (expected 5,5)" << std::endl;

    // -- from_bounds --
    auto box = Polygon<double>::from_bounds(0, 0, 10, 10);
    std::cout << "from_bounds(0,0,10,10).area() = " << box.area() << " (expected 100.0)" << std::endl;

    // -- float32 Polygon --
    float f32_sq[] = {0,0, 10,0, 10,10, 0,10};
    Polygon<float> fpoly(f32_sq, 4);
    std::cout << "\n[float32] fpoly.area() = " << fpoly.area() << " (expected 100.0)" << std::endl;
    std::cout << "fpoly.is_valid() = " << fpoly.is_valid() << std::endl;
}

// ===========================================================================
// 4. Cross-type operations
// ===========================================================================
void demo_cross_type() {
    section("4. Cross-type operations");

    // --- Point ↔ LineString ---
    double ls_coords[] = {0,0, 10,0};
    LineString<double> ls(ls_coords, 2);

    Point<double> pt_on(5, 0), pt_off(5, 5), pt_end(10, 0);

    std::cout << "ls.distance(pt_on)  = " << ls.distance(pt_on)  << " (expected 0.0)" << std::endl;
    std::cout << "ls.distance(pt_off) = " << ls.distance(pt_off) << " (expected 5.0)" << std::endl;
    std::cout << "pt_on.distance(ls)  = " << pt_on.distance(ls)  << " (symmetric, expected 0.0)" << std::endl;
    std::cout << "ls.intersects(pt_on) = " << ls.intersects(pt_on) << " (expected 1)" << std::endl;
    std::cout << "ls.intersects(pt_off) = " << ls.intersects(pt_off) << " (expected 0)" << std::endl;
    std::cout << "ls.contains(pt_on) = " << ls.contains(pt_on) << " (expected 1)" << std::endl;

    // --- Point ↔ Polygon ---
    double sq_c[] = {0,0, 10,0, 10,10, 0,10};
    Polygon<double> sq(sq_c, 4);
    Point<double> pt_inside(5, 5), pt_outside(20, 20), pt_edge(0, 5);

    std::cout << "\nsq.contains(pt_inside) = " << sq.contains(pt_inside) << " (expected 1)" << std::endl;
    std::cout << "sq.contains(pt_outside) = " << sq.contains(pt_outside) << " (expected 0)" << std::endl;
    std::cout << "sq.touches(pt_edge) = " << sq.touches(pt_edge) << " (expected 1)" << std::endl;
    std::cout << "sq.distance(pt_outside) = " << sq.distance(pt_outside) << " (expected ~14.14)" << std::endl;
    std::cout << "pt_inside.within(sq) = " << pt_inside.within(sq) << " (expected 1)" << std::endl;
    std::cout << "pt_inside.intersects(sq) = " << pt_inside.intersects(sq) << " (expected 1)" << std::endl;

    // --- LineString ↔ Polygon ---
    double lcross[] = {-5,5, 15,5};
    LineString<double> lc(lcross, 2);
    double lout[] = {20,0, 25,0};
    LineString<double> lo(lout, 2);

    std::cout << "\nlc.distance(sq) = " << lc.distance(sq) << " (crossing, expected 0.0)" << std::endl;
    std::cout << "lc.intersects(sq) = " << lc.intersects(sq) << " (expected 1)" << std::endl;
    std::cout << "lo.distance(sq) = " << lo.distance(sq) << " (expected 10.0)" << std::endl;
    std::cout << "sq.intersects(lo) = " << sq.intersects(lo) << " (expected 0)" << std::endl;

    // --- project / interpolate ---
    double proj = lc.project(Point<double>(10, 5));
    std::cout << "\nlc.project(Point(10,5)) = " << proj << " (expected ~15.0)" << std::endl;
    auto interp = lc.interpolate(10.0);
    std::cout << "lc.interpolate(10.0) = (" << interp.x << ", " << interp.y << ") (expected ~5,5)" << std::endl;
}

// ===========================================================================
// 5. Collision detection patterns (from production code)
// ===========================================================================
void demo_collision() {
    section("5. Collision detection patterns");

    // Pattern 1: Polygon-Polygon distance → collision = (distance == 0.0)
    {
        double a[] = {0,0, 10,0, 10,10, 0,10};
        double b[] = {5,0, 15,0, 15,10, 5,10};
        Polygon<double> pa(a, 4), pb(b, 4);
        double d = pa.distance(pb);
        bool collision = (d == 0.0);
        std::cout << "[Pattern 1] Poly-Poly distance=" << d
                  << " collision=" << collision << " (overlapping → expected 0.0, true)" << std::endl;
    }

    // Pattern 2: Polygon-LineString distance
    {
        double sq[] = {0,0, 10,0, 10,10, 0,10};
        double line[] = {15,5, 20,5};
        Polygon<double> p(sq, 4);
        LineString<double> l(line, 2);
        double d = p.distance(l);
        std::cout << "[Pattern 2] Poly-LS distance=" << d << " (expected 5.0)" << std::endl;
    }

    // Pattern 3: intersects + distance < 1e-12
    {
        double a[] = {0,0, 10,0, 10,10, 0,10};
        double b[] = {10.0 + 1e-12, 0, 20, 0, 20, 10, 10.0 + 1e-12, 10};
        Polygon<double> pa(a, 4), pb(b, 4);
        bool intr = pa.intersects(pb);
        double d = pa.distance(pb);
        bool near = intr && (d < 1e-12);
        std::cout << "[Pattern 3] intersects=" << intr << " distance=" << d
                  << " near_collision=" << near << " (ε-gap → expected false)" << std::endl;
    }

    // Pattern 4: LineString-Point distance (ego distance)
    {
        double line[] = {0,0, 100,0};
        LineString<double> l(line, 2);
        Point<double> ego(30, 0);
        double dist = l.distance(ego);
        std::cout << "[Pattern 4] LS-Point distance(ego) = " << dist
                  << " (point on line → expected 0.0)" << std::endl;
    }

    // Pattern 5: LineString-LineString distance → half_lane_width
    {
        double l1[] = {0,0, 100,0};
        double l2[] = {0,3.5, 100,3.5};
        LineString<double> line1(l1, 2), line2(l2, 2);
        double lane_dist = line1.distance(line2);
        double half = lane_dist / 2.0;
        std::cout << "[Pattern 5] LS-LS distance=" << lane_dist
                  << " half_lane_width=" << half << " (expected 3.5, 1.75)" << std::endl;
    }

    // Pattern 6: LS↔Poly / Poly↔LS (both directions)
    {
        double sq[] = {0,0, 10,0, 10,10, 0,10};
        double cl[] = {5,-5, 5,15};  // centerline crosses polygon
        Polygon<double> p(sq, 4);
        LineString<double> centerline(cl, 2);
        double d1 = centerline.distance(p);  // LS → Poly
        double d2 = p.distance(centerline);   // Poly → LS
        std::cout << "[Pattern 6] LS→Poly=" << d1 << " Poly→LS=" << d2
                  << " (crossing → expected 0.0 both, symmetric)" << std::endl;
    }
}

// ===========================================================================
// 6. LinearRing
// ===========================================================================
void demo_linearring() {
    section("6. LinearRing");

    double ccw[] = {0,0, 10,0, 10,10, 0,10};
    double cw[]  = {0,0, 0,10, 10,10, 10,0};  // reversed
    double open_ccw[] = {0,0, 10,0, 10,10};     // not closed

    LinearRing<double> ring_ccw(ccw, 4);
    LinearRing<double> ring_cw(cw, 4);
    LinearRing<double> ring_open(open_ccw, 3);   // auto-closed by constructor
    LinearRing<double> empty_ring;                // default empty

    // -- properties --
    std::cout << "ring_ccw.is_closed() = " << ring_ccw.is_closed() << " (expected 1)" << std::endl;
    std::cout << "ring_ccw.is_ring() = " << ring_ccw.is_ring() << " (expected 1)" << std::endl;
    std::cout << "ring_ccw.is_ccw() = " << ring_ccw.is_ccw() << " (expected 1)" << std::endl;
    std::cout << "ring_cw.is_ccw() = " << ring_cw.is_ccw() << " (expected 0)" << std::endl;
    std::cout << "ring_open.is_closed() = " << ring_open.is_closed() << " (auto-closed, expected 1)" << std::endl;
    std::cout << "ring_ccw.length() = " << ring_ccw.length() << " (expected 40.0)" << std::endl;
    std::cout << "ring_ccw.area() = " << ring_ccw.area() << " (LinearRing area always 0.0)" << std::endl;
    std::cout << "empty_ring.is_empty() = " << empty_ring.is_empty() << " (expected 1)" << std::endl;

    // -- WKT --
    std::cout << "\nring_ccw.wkt() = " << ring_ccw.wkt() << std::endl;

    // -- centroid --
    auto cent = ring_ccw.centroid();
    std::cout << "ring_ccw.centroid() = (" << cent.x << ", " << cent.y << ") (expected 5,5)" << std::endl;

    // -- bounds --
    auto bounds = ring_ccw.bounds();
    std::cout << "ring_ccw.bounds() = [" << bounds[0] << ", " << bounds[1]
              << ", " << bounds[2] << ", " << bounds[3] << "]" << std::endl;
}

// ===========================================================================
// 7. nearest_points
// ===========================================================================
void demo_nearest_points() {
    section("7. nearest_points (shapely.ops)");

    // Poly ↔ LS
    double sq_c[] = {0,0, 10,0, 10,10, 0,10};
    double ls_c[] = {15,5, 25,5};
    Polygon<double> poly(sq_c, 4);
    LineString<double> ls(ls_c, 2);

    auto [px, py, lx, ly] = nearest_points(poly, ls);
    std::cout << "nearest_points(poly, ls) → poly=(" << px << ", " << py
              << ") line=(" << lx << ", " << ly << ")" << std::endl;
    std::cout << "  (expect poly(10,5) → line(15,5), distance 5.0)" << std::endl;

    // LS ↔ Point
    double line_c[] = {0,0, 10,0};
    LineString<double> line(line_c, 2);
    Point<double> pt(5, 3);

    auto [lx2, ly2, ptx2, pty2] = nearest_points(line, pt);
    std::cout << "\nnearest_points(line, point) → line=(" << lx2 << ", " << ly2
              << ") point=(" << ptx2 << ", " << pty2 << ")" << std::endl;
    std::cout << "  (expect line(5,0) → point(5,3), distance 3.0)" << std::endl;
}

// ===========================================================================
// 8. WKT / WKB serialization
// ===========================================================================
void demo_serialization() {
    section("8. Serialization (WKT / WKB)");

    Point<double> pt(3.14, -2.71);
    std::cout << "Point WKT:    " << pt.wkt() << std::endl;
    std::cout << "Point WKB_hex: " << pt.wkb_hex() << std::endl;
    std::cout << "Point geom_type: " << pt.geom_type() << std::endl;

    double ls_c[] = {0,0, 10,10};
    LineString<double> ls(ls_c, 2);
    std::cout << "LineString WKT: " << ls.wkt() << std::endl;

    double sq_c[] = {0,0, 10,0, 10,10, 0,10};
    Polygon<double> sq(sq_c, 4);
    std::cout << "Polygon WKT:    " << sq.wkt() << std::endl;
}

// ===========================================================================
// 9. Float32 precision demo
// ===========================================================================
void demo_float32() {
    section("9. Float32 precision");

    // float32 types use the same GEOS engine — identical results when inputs match
    Point<float> fp1(0.0f, 0.0f);
    Point<float> fp2(3.0f, 4.0f);
    std::cout << "float32 Point distance: " << fp1.distance(fp2)
              << " (expected 5.0 — exact)" << std::endl;

    // float32 coordinates have ~7 significant digits
    Point<float> fp3(1.0f / 3.0f, 2.0f / 3.0f);
    std::cout << "fp3 = (" << fp3.x << ", " << fp3.y << ")"
              << "  ← f32 representation of 1/3, 2/3" << std::endl;

    // float32 Polygon
    float tri[] = {0,0, 10,0, 0,10};
    Polygon<float> ftri(tri, 3);
    std::cout << "float32 triangle area: " << ftri.area() << " (expected 50.0)" << std::endl;

    // float32 LineString
    float f32_ls[] = {0,0, 100,0};
    LineString<float> fls(f32_ls, 2);
    std::cout << "float32 LineString length: " << fls.length() << " (expected 100.0)" << std::endl;
}

// ===========================================================================
// main
// ===========================================================================
int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "shapelycpp — Comprehensive C++ Example Suite\n";
    std::cout << "============================================\n";

    demo_point();
    demo_linestring();
    demo_polygon();
    demo_cross_type();
    demo_collision();
    demo_linearring();
    demo_nearest_points();
    demo_serialization();
    demo_float32();

    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  All examples completed successfully!" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    return 0;
}
