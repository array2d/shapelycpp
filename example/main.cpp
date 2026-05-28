// shapelycpp pure native C++ usage example (zero pybind11 dependency)

#include <iostream>
#include "shapely/geometry/point.h"
#include "shapely/geometry/linestring.h"
#include "shapely/geometry/polygon.h"
#include "shapely/geometry/linearring.h"

using namespace shapely::geometry;

int main() {
    // -- Point --
    Point<double> p1(0.0, 0.0), p2(3.0, 4.0);
    std::cout << "Point distance: " << p1.distance(p2) << " (expected 5.0)" << std::endl;
    std::cout << "Point WKT: " << p1.wkt() << std::endl;
    std::cout << "Point is_valid: " << p1.is_valid() << std::endl;
    std::cout << "Point bounds: [" << p1.bounds()[0] << ", " << p1.bounds()[1]
              << ", " << p1.bounds()[2] << ", " << p1.bounds()[3] << "]" << std::endl;

    // -- Buffer --
    auto buf = p1.buffer(10.0);
    std::cout << "Buffer area: " << buf.area() << std::endl;
    std::cout << "Buffer WKT: " << buf.wkt() << std::endl;

    // -- Predicates --
    double sq_coords[] = {0,0, 10,0, 10,10, 0,10};
    Polygon<double> sq(sq_coords, 4, 2);
    Point<double> inside(5, 5), outside(20, 20);
    std::cout << "Square contains inside point: " << sq.contains(inside) << " (expected 1)" << std::endl;
    std::cout << "Square contains outside point: " << sq.contains(outside) << " (expected 0)" << std::endl;
    std::cout << "Square disjoint outside point: " << sq.disjoint(outside) << " (expected 1)" << std::endl;

    // -- LineString --
    double ls_coords[] = {0,0, 10,0};
    LineString<double> ls(ls_coords, 2);
    std::cout << "LineString length: " << ls.length() << " (expected 10.0)" << std::endl;
    std::cout << "LineString is_closed: " << ls.is_closed() << " (expected 0)" << std::endl;
    std::cout << "LineString WKT: " << ls.wkt() << std::endl;

    // -- intersects --
    double tri_coords[] = {5,-5, 15,0, 5,5};
    Polygon<double> tri(tri_coords, 3, 2);
    std::cout << "LineString intersects triangle: " << ls.intersects(tri) << " (expected 1)" << std::endl;

    // -- LinearRing --
    double ring_coords[] = {0,0, 10,0, 10,10, 0,10};
    LinearRing<double> ring(ring_coords, 4);
    std::cout << "LinearRing is_closed: " << ring.is_closed() << " (expected 1)" << std::endl;
    std::cout << "LinearRing is_ccw: " << ring.is_ccw() << std::endl;
    std::cout << "LinearRing length: " << ring.length() << std::endl;
    std::cout << "LinearRing WKT: " << ring.wkt() << std::endl;

    // -- Polygon exterior --
    auto exterior = sq.exterior();
    std::cout << "Polygon exterior rows: " << exterior.rows() << " (expected 5)" << std::endl;

    std::cout << "\nAll examples passed!" << std::endl;
    return 0;
}
