// Python Source: shapely/geometry/polygon.py
// Line Range: class Polygon(BaseGeometry) (L18-L1095)
// Alignment: strict
// EXEMPTION: cpp_template_optimization
// Reason: C++ template to support both float32 and double coordinates.

#pragma once

#include <memory>
#include <vector>
#include <cstddef>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>

namespace shapely {
namespace geometry {

#ifndef SHAPELY_GEOMETRY_LINESTRING_DEFINED
template <typename T>
class LineString;
#endif

template <typename T = double>
class Polygon {
public:
    /// Empty polygon
    Polygon();

    /// Construct from raw coordinate array [rows x cols]
    Polygon(const T* coords, size_t rows, size_t cols = 2);

    Polygon(Polygon&&) = default;
    Polygon& operator=(Polygon&&) = default;

    /// Raw coordinate access
    const T* data() const { return coords_.data(); }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    double area() const;
    double distance(const Polygon& other) const;

    template <typename U>
    double distance(const LineString<U>& other) const;

    bool intersects(const Polygon& other) const;

    template <typename U>
    bool intersects(const LineString<U>& other) const;

    Polygon<double> intersection(const Polygon<double>& other) const;
    bool is_valid() const;
    bool is_empty() const;
    Polygon<double> buffer(double distance) const;

private:
    template <typename U> friend class Polygon;
    template <typename U> friend class LineString;
    template <typename U> friend class Point;

    std::vector<T> coords_;
    size_t rows_ = 0;
    size_t cols_ = 0;
    std::unique_ptr<geos::geom::Polygon> geos_polygon_;
    geos::geom::GeometryFactory::Ptr factory_;
};

} // namespace geometry
} // namespace shapely

#define SHAPELY_GEOMETRY_POLYGON_DEFINED

// ============================================================================
// Implementation — requires full LineString definition
// ============================================================================

#include "shapely/geometry/linestring.h"

#include <geos/geom/LineString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/util/TopologyException.h>
#include <stdexcept>

namespace shapely {
namespace geometry {

// Python: shapely/geometry/polygon.py::Polygon.__init__:L1095

template <typename T>
Polygon<T>::Polygon()
{
    factory_ = geos::geom::GeometryFactory::create();
    geos_polygon_ = factory_->createPolygon();
}

template <typename T>
Polygon<T>::Polygon(const T* coords, size_t rows, size_t cols)
    : coords_(coords, coords + rows * cols), rows_(rows), cols_(cols)
{
    factory_ = geos::geom::GeometryFactory::create();
    if (rows < 3 || cols < 2)
    {
        geos_polygon_ = factory_->createPolygon();
        return;
    }

    auto coord_seq = factory_->getCoordinateSequenceFactory()->create(rows + 1, 2);
    for (size_t i = 0; i < rows; ++i)
        coord_seq->setAt(geos::geom::Coordinate(
            static_cast<double>(coords_[i * cols + 0]),
            static_cast<double>(coords_[i * cols + 1])), i);
    coord_seq->setAt(geos::geom::Coordinate(
        static_cast<double>(coords_[0]),
        static_cast<double>(coords_[1])), rows);

    auto ring = factory_->createLinearRing(std::move(coord_seq));
    geos_polygon_ = factory_->createPolygon(std::move(ring));
}

// Python: shapely/geometry/polygon.py::Polygon.area:L500

template <typename T>
double Polygon<T>::area() const
{
    return geos_polygon_->getArea();
}

// Python: shapely/geometry/polygon.py::Polygon.distance:L530

template <typename T>
double Polygon<T>::distance(const Polygon& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_polygon_.get(), other.geos_polygon_.get());
    return dist_op.distance();
}

// Python: shapely/geometry/polygon.py::Polygon.distance:L530

template <typename T>
template <typename U>
double Polygon<T>::distance(const LineString<U>& other) const
{
    geos::operation::distance::DistanceOp dist_op(geos_polygon_.get(), other.geos_linestring_.get());
    return dist_op.distance();
}

// Python: shapely/geometry/polygon.py::Polygon.intersects:L567

template <typename T>
bool Polygon<T>::intersects(const Polygon& other) const
{
    return geos_polygon_->intersects(other.geos_polygon_.get());
}

// Python: shapely/geometry/polygon.py::Polygon.intersects:L567

template <typename T>
template <typename U>
bool Polygon<T>::intersects(const LineString<U>& other) const
{
    return geos_polygon_->intersects(other.geos_linestring_.get());
}

// Python: shapely/geometry/polygon.py::Polygon.intersection:L544

template <typename T>
Polygon<double> Polygon<T>::intersection(const Polygon<double>& other) const
{
    if (geos_polygon_->isEmpty() || other.geos_polygon_->isEmpty())
        return Polygon<double>();

    std::unique_ptr<geos::geom::Geometry> inter;
    if (!geos_polygon_->isValid() || !other.geos_polygon_->isValid()) {
        return Polygon<double>();
    }
    inter = geos_polygon_->intersection(other.geos_polygon_.get());
    if (inter == nullptr || inter->isEmpty())
        return Polygon<double>();

    const geos::geom::Geometry *geom = inter.get();
    if (geom->getGeometryTypeId() == geos::geom::GEOS_POLYGON)
    {
        const geos::geom::Polygon *geos_poly = dynamic_cast<const geos::geom::Polygon *>(geom);
        if (geos_poly)
        {
            const geos::geom::CoordinateSequence *cs = geos_poly->getExteriorRing()->getCoordinatesRO();
            if (cs && !cs->isEmpty())
            {
                size_t coord_n = cs->getSize();
                std::vector<double> coords(coord_n * 2);
                for (size_t i = 0; i < coord_n; ++i) {
                    coords[i * 2]     = cs->getAt(i).x;
                    coords[i * 2 + 1] = cs->getAt(i).y;
                }
                return Polygon<double>(coords.data(), coord_n, 2);
            }
        }
    }

    return Polygon<double>();
}

// Python: shapely/geometry/polygon.py::Polygon.is_valid:L755

template <typename T>
bool Polygon<T>::is_valid() const
{
    if (geos_polygon_->isEmpty())
        return false;
    return geos_polygon_->isValid();
}

// Python: shapely/geometry/polygon.py::Polygon.is_empty:L715

template <typename T>
bool Polygon<T>::is_empty() const
{
    return geos_polygon_->isEmpty();
}

// Python: shapely/geometry/polygon.py::Polygon.buffer:L436

template <typename T>
Polygon<double> Polygon<T>::buffer(double distance) const
{
    if (geos_polygon_->isEmpty())
        return Polygon<double>();

    std::unique_ptr<geos::geom::Geometry> buf_geom;
    if (!geos_polygon_->isValid() || !geos_polygon_->isSimple()) {
        return Polygon<double>();
    }
    buf_geom = geos_polygon_->buffer(distance, 16);
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

} // namespace geometry
} // namespace shapely
