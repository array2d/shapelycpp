// Python Source: shapely/geometry/linestring.py (LinearRing extends LineString)
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template for float32/float64 coordinate support.

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <tuple>
#include <cstddef>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>

namespace shapely {
namespace geometry {

template <typename T = double>
class LinearRing {
public:
    /// Empty ring
    LinearRing();

    /// Construct from raw coordinate array [rows x cols]. Ring will be auto-closed.
    LinearRing(const T* coords, size_t rows, size_t cols = 2);

    LinearRing(LinearRing&&) = default;
    LinearRing& operator=(LinearRing&&) = default;

    // -- Raw coordinate access --
    const T* data() const { return coords_.data(); }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    // -- coords, xy --
    std::vector<std::tuple<T, T>> coords() const;
    std::tuple<std::vector<T>, std::vector<T>> xy() const;

    // -- Properties --
    bool is_empty() const;
    bool is_simple() const;
    bool is_valid() const;
    bool is_closed() const;
    bool is_ring() const;
    bool is_ccw() const;
    double area() const;
    double length() const;
    std::vector<double> bounds() const;

    // -- Accessors --
    std::string wkt() const;
    std::string wkb_hex() const;
    std::string type() const;
    std::string geom_type() const;
    bool has_z() const;

    // -- Methods --
    Point<double> centroid() const;
    void normalize();

private:
    template <typename U> friend class Polygon;
    template <typename U> friend class LineString;
    std::vector<T> coords_;
    size_t rows_ = 0, cols_ = 0;
    std::unique_ptr<geos::geom::LinearRing> geos_ring_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_LINEARRING_DEFINED

// ============================================================================
// Implementation
// ============================================================================

#include "shapely/geometry/point.h"
#include "shapely/detail/geos_utils.h"

#include <geos/geom/Point.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/algorithm/Orientation.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

template <typename T>
LinearRing<T>::LinearRing() {
    factory_ = geos::geom::GeometryFactory::create();
    auto cs = factory_->getCoordinateSequenceFactory()->create(std::size_t(0), std::size_t(2));
    geos_ring_ = factory_->createLinearRing(std::move(cs));
}

template <typename T>
LinearRing<T>::LinearRing(const T* coords, size_t rows, size_t cols)
    : coords_(coords, coords + rows * cols), rows_(rows), cols_(cols)
{
    factory_ = geos::geom::GeometryFactory::create();
    if (rows < 3 || cols < 2) {
        auto cs = factory_->getCoordinateSequenceFactory()->create(std::size_t(0), std::size_t(2));
        geos_ring_ = factory_->createLinearRing(std::move(cs));
        return;
    }

    // Check if already closed; if not, close it
    bool already_closed = (coords_[0] == coords_[(rows-1)*cols] && coords_[1] == coords_[(rows-1)*cols+1]);
    size_t crd_n = already_closed ? rows : rows + 1;

    auto cs = factory_->getCoordinateSequenceFactory()->create(crd_n, 2);
    for (size_t i = 0; i < rows; ++i)
        cs->setAt(geos::geom::Coordinate(static_cast<double>(coords_[i*cols]),
                                          static_cast<double>(coords_[i*cols+1])), i);
    if (!already_closed)
        cs->setAt(geos::geom::Coordinate(static_cast<double>(coords_[0]),
                                          static_cast<double>(coords_[1])), rows);

    geos_ring_ = factory_->createLinearRing(std::move(cs));
}

// -- coords, xy -------------------------------------------------------------

template <typename T>
std::vector<std::tuple<T, T>> LinearRing<T>::coords() const {
    std::vector<std::tuple<T, T>> r; r.reserve(rows_);
    for (size_t i = 0; i < rows_; ++i) r.emplace_back(coords_[i*cols_], coords_[i*cols_+1]);
    return r;
}

template <typename T>
std::tuple<std::vector<T>, std::vector<T>> LinearRing<T>::xy() const {
    std::vector<T> xs(rows_), ys(rows_);
    for (size_t i = 0; i < rows_; ++i) { xs[i]=coords_[i*cols_]; ys[i]=coords_[i*cols_+1]; }
    return {xs, ys};
}

// -- Properties --------------------------------------------------------------

template <typename T> bool LinearRing<T>::is_empty() const { return detail::geos_is_empty(geos_ring_.get()); }
template <typename T> bool LinearRing<T>::is_simple() const { return detail::geos_is_simple(geos_ring_.get()); }
template <typename T> bool LinearRing<T>::is_valid() const { return detail::geos_is_valid(geos_ring_.get()); }
template <typename T> bool LinearRing<T>::is_closed() const { return geos_ring_->isClosed(); }
template <typename T> bool LinearRing<T>::is_ring() const { return geos_ring_->isRing(); }

template <typename T>
bool LinearRing<T>::is_ccw() const {
    if (rows_ < 3 || !geos_ring_) return false;
    return geos::algorithm::Orientation::isCCW(geos_ring_->getCoordinatesRO());
}

template <typename T> double LinearRing<T>::area() const { return 0.0; }
template <typename T> double LinearRing<T>::length() const { return geos_ring_->getLength(); }
template <typename T> std::vector<double> LinearRing<T>::bounds() const { return detail::geos_bounds(geos_ring_.get()); }

// -- Accessors ---------------------------------------------------------------

template <typename T> std::string LinearRing<T>::wkt() const { return detail::geos_to_wkt(geos_ring_.get()); }
template <typename T> std::string LinearRing<T>::wkb_hex() const { return detail::geos_to_wkb_hex(geos_ring_.get()); }
template <typename T> std::string LinearRing<T>::type() const { return "LinearRing"; }
template <typename T> std::string LinearRing<T>::geom_type() const { return detail::geos_geom_type(geos_ring_.get()); }
template <typename T> bool        LinearRing<T>::has_z() const { return detail::geos_has_z(geos_ring_.get()); }

// -- centroid / normalize ----------------------------------------------------

template <typename T>
Point<double> LinearRing<T>::centroid() const {
    auto c = geos_ring_->getCentroid();
    if (!c) return Point<double>(0, 0);
    auto* coord = c->getCoordinate();
    return Point<double>(coord->x, coord->y);
}

template <typename T>
void LinearRing<T>::normalize() { geos_ring_->normalize(); }

} // namespace geometry
} // namespace shapely
