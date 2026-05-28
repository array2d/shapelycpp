// Python Source: shapely/geometry/linestring.py
// Line Range: class LineString(BaseGeometry) (L18-L1070)
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template to support both float32 and double coordinates.

#pragma once

#include <memory>
#include <vector>
#include <cstddef>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_POLYGON_DEFINED
template <typename T>
class Polygon;
#endif

#ifndef SHAPELY_GEOMETRY_POINT_DEFINED
template <typename T>
class Point;
#endif

template <typename T = double>
class LineString {
public:
    /// Construct from raw coordinate array [rows x cols]
    LineString(const T* coords, size_t rows, size_t cols = 2);

    LineString(LineString&&) = default;
    LineString& operator=(LineString&&) = default;

    /// Raw coordinate access
    const T* data() const { return coords_.data(); }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    double distance(const LineString& other) const;
    template <typename U>
    double distance(const Polygon<U>& other) const;

    template <typename U>
    double distance(const Point<U>& other) const;

    template <typename U>
    bool intersects(const Polygon<U>& other) const;

    template <typename U>
    double project(const Point<U>& other) const;

    Point<double> interpolate(double distance) const;

    Polygon<double> buffer(double distance) const;

    double length() const;

private:
    template <typename U> friend class Polygon;
    template <typename U> friend class Point;
    std::vector<T> coords_;
    size_t rows_ = 0;
    size_t cols_ = 0;
    std::unique_ptr<geos::geom::LineString> geos_linestring_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_LINESTRING_DEFINED

// ============================================================================
// Implementation — requires full Point and Polygon definitions
// ============================================================================

#include "shapely/geometry/point.h"
#include "shapely/geometry/polygon.h"

#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/linearref/LengthIndexedLine.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

// Python: shapely/geometry/linestring.py::LineString.__init__:L1070

template <typename T>
LineString<T>::LineString(const T* coords, size_t rows, size_t cols)
    : coords_(coords, coords + rows * cols), rows_(rows), cols_(cols)
{
    if (cols < 2)
        throw std::runtime_error("LineString: coords must have at least 2 columns");
    if (rows < 2)
        throw std::runtime_error("LineString: must have at least 2 points");

    factory_ = geos::geom::GeometryFactory::create();

    auto coord_seq = factory_->getCoordinateSequenceFactory()->create(rows, 2);
    for (size_t i = 0; i < rows; ++i)
        coord_seq->setAt(geos::geom::Coordinate(
            static_cast<double>(coords_[i * cols + 0]),
            static_cast<double>(coords_[i * cols + 1])), i);

    geos_linestring_ = factory_->createLineString(std::move(coord_seq));
}

// Python: shapely/geometry/linestring.py::LineString.distance:L500

template <typename T>
double LineString<T>::distance(const LineString& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_linestring_.get(), other.geos_linestring_.get());
    return dist_op.distance();
}

// Python: shapely/geometry/linestring.py::LineString.distance:L500

template <typename T>
template <typename U>
double LineString<T>::distance(const Polygon<U>& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_linestring_.get(), other.geos_polygon_.get());
    return dist_op.distance();
}

// Python: shapely/geometry/linestring.py::LineString.distance:L500

template <typename T>
template <typename U>
double LineString<T>::distance(const Point<U>& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_linestring_.get(), other.geos_point_.get());
    return dist_op.distance();
}

// Python: shapely/geometry/linestring.py::LineString.intersects:L540

template <typename T>
template <typename U>
bool LineString<T>::intersects(const Polygon<U>& other) const
{
    return geos_linestring_->intersects(other.geos_polygon_.get());
}

// Python: shapely/geometry/linestring.py::LineString.project:L400

template <typename T>
template <typename U>
double LineString<T>::project(const Point<U>& other) const
{
    geos::linearref::LengthIndexedLine indexed_line(geos_linestring_.get());
    return indexed_line.project(geos::geom::Coordinate(
        static_cast<double>(other.x), static_cast<double>(other.y)));
}

// Python: shapely/geometry/linestring.py::LineString.interpolate:L440

template <typename T>
Point<double> LineString<T>::interpolate(double distance) const
{
    geos::linearref::LengthIndexedLine indexed_line(geos_linestring_.get());
    auto coord = indexed_line.extractPoint(distance);
    return Point<double>(coord.x, coord.y);
}

// Python: shapely/geometry/linestring.py::LineString.buffer:L580

template <typename T>
Polygon<double> LineString<T>::buffer(double distance) const
{
    if (!geos_linestring_ || geos_linestring_->isEmpty()) {
        return Polygon<double>();
    }
    auto buf_geom = geos_linestring_->buffer(distance, 16);
    if (buf_geom == nullptr || buf_geom->isEmpty())
        return Polygon<double>();

    const geos::geom::Geometry *poly = buf_geom.get();
    if (poly->getGeometryTypeId() != geos::geom::GEOS_POLYGON)
    {
        if (poly->getNumGeometries() > 0)
            poly = poly->getGeometryN(0);
    }

    if (poly->getGeometryTypeId() != geos::geom::GEOS_POLYGON || poly->isEmpty())
        return Polygon<double>();

    const geos::geom::Polygon *geos_poly = dynamic_cast<const geos::geom::Polygon *>(poly);
    if (!geos_poly)
        return Polygon<double>();

    const geos::geom::CoordinateSequence *cs = geos_poly->getExteriorRing()->getCoordinatesRO();
    if (!cs || cs->isEmpty())
        return Polygon<double>();

    size_t coord_n = cs->getSize();
    std::vector<double> coords(coord_n * 2);
    for (size_t i = 0; i < coord_n; ++i) {
        coords[i * 2]     = cs->getAt(i).x;
        coords[i * 2 + 1] = cs->getAt(i).y;
    }

    return Polygon<double>(coords.data(), coord_n, 2);
}

// Python: shapely/geometry/linestring.py::LineString.length:L600

template <typename T>
double LineString<T>::length() const
{
    return geos_linestring_->getLength();
}

} // namespace geometry
} // namespace shapely
