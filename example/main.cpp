#include <iostream>
#include "shapely/geometry/point.h"
#include "shapely/geometry/linestring.h"

int main() {
    // Point operations
    shapely::geometry::Point<double> p1(0.0, 0.0);
    shapely::geometry::Point<double> p2(3.0, 4.0);

    double d = p1.distance(p2);
    std::cout << "Distance: " << d << " (expected 5.0)" << std::endl;

    // Buffer
    auto buffered = p1.buffer(10.0);
    std::cout << "Buffer created OK" << std::endl;

    return 0;
}
