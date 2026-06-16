"""
Full 0-ULP bit-level alignment test for shapelycpp C++ vs Python shapely.

Run:  PYTHONPATH=build/tests pytest tests/test_all.py -v

Architecture: 5 functions driving all tests (following numpypcpp pattern).
  F3  compare()      -- bit-exact 0-ULP comparison (view-as-uint, hex dump)
  F4  call_cpp_py()  -- dispatch C++ to Python shapely equivalents by api_name
  F5  api_catalog()  -- export full API metadata (5 groups)
  F1/F2 extreme data / reshape -- embedded in catalog factory functions

Design principle:
  C++ and Python shapely link the SAME GEOS library. Input coordinates are
  identical doubles -> GEOS produces identical output -> 0-ULP alignment is
  mandatory.  NO tolerance/epsilon comparisons.  If there is a mismatch,
  it is a REAL BUG and the test MUST FAIL.
"""

import os, importlib, re
import numpy as np
import pytest
from collections import namedtuple
from shapely.geometry import (
    Point as PyPoint, LineString as PyLineString,
    Polygon as PyPolygon, LinearRing as PyLinearRing,
    MultiPoint as PyMultiPoint, MultiLineString as PyMultiLineString,
    MultiPolygon as PyMultiPolygon,
)
import shapely.ops as py_ops
from shapely import wkt as shapely_wkt


# =============================================================================
# F3: compare -- 0-ULP bit-level comparison (from numpypcpp)
# =============================================================================

_UINT_VIEW = {4: np.uint32, 8: np.uint64}
_EL_VIEW  = {2: np.uint16, 4: np.uint32, 8: np.uint64}
_EL_FMT   = {2: "04x", 4: "08x", 8: "016x"}


def compare(cpp_result, py_result, strategy="bit_exact", label=""):
    """Unified comparison entry.  All strategies are 0-ULP strict.

    bit_exact   -- uint-view bit-level equality (default, for arrays)
    scalar_eq   -- exact float equality (a == b, for GEOS scalars)
    bool_eq     -- exact bool equality (a == b)
    str_eq      -- exact string equality (a == b, for WKT/WKB)
    """
    if strategy == "bool_eq":
        if bool(cpp_result) != bool(py_result):
            raise AssertionError(
                f"[{label}] bool mismatch: C++={cpp_result} vs shapely={py_result}")
        return
    if strategy == "scalar_eq":
        c = _to_float(cpp_result)
        p = _to_float(py_result)
        if c != p:
            raise AssertionError(
                f"[{label}] scalar mismatch: C++={c:.16e} vs shapely={p:.16e}")
        return
    if strategy == "str_eq":
        if str(cpp_result) != str(py_result):
            raise AssertionError(
                f"[{label}] str mismatch:\n  C++: {cpp_result}\n  py:  {py_result}")
        return
    # bit_exact
    _compare_bit_exact(cpp_result, py_result, label)


def _to_float(v):
    try:
        return float(np.asarray(v).flat[0])
    except (TypeError, ValueError, IndexError):
        return float(v)


def _compare_bit_exact(cpp_result, py_result, label=""):
    cpp = np.asarray(cpp_result, dtype=np.float64)
    py  = np.asarray(py_result, dtype=np.float64)

    if cpp.shape != py.shape:
        raise AssertionError(
            f"[{label}] shape mismatch: C++ {cpp.shape} vs shapely {py.shape}")

    uint_t = _UINT_VIEW.get(cpp.itemsize)
    if uint_t is not None:
        cpp_u = np.ascontiguousarray(cpp).ravel().view(uint_t)
        py_u  = np.ascontiguousarray(py).ravel().view(uint_t)
        if np.array_equal(cpp_u, py_u):
            return
        diff_mask = (cpp_u != py_u).reshape(cpp.shape)
    else:
        if np.array_equal(cpp, py):
            return
        diff_mask = cpp != py

    n_diff = int(np.sum(diff_mask))
    diff_idx = np.flatnonzero(diff_mask.ravel())
    lines = [f"[{label}] BIT-LEVEL MISMATCH: {n_diff}/{cpp.size}"]
    for idx in diff_idx[:5]:
        cv, pv = cpp.flat[idx], py.flat[idx]
        cvt = _EL_VIEW.get(cpp.itemsize)
        pvt = _EL_VIEW.get(py.itemsize)
        cf  = _EL_FMT.get(cpp.itemsize, "016x")
        pf  = _EL_FMT.get(py.itemsize, "016x")
        ch = np.ascontiguousarray(cpp).view(cvt).flat[idx] if cvt else 0
        ph = np.ascontiguousarray(py).view(pvt).flat[idx] if pvt else 0
        lines.append(f"  [{idx}] C++={cv:.16e} (0x{ch:{cf}})  vs  "
                     f"shapely={pv:.16e} (0x{ph:{pf}})")
    if len(diff_idx) > 5:
        lines.append(f"  ... and {len(diff_idx) - 5} more diffs")
    raise AssertionError("\n".join(lines))


# =============================================================================
# F4: call_cpp_py -- dispatch C++ and Python shapely equivalents
# =============================================================================

# Type tag -> C++ factory function name / Python constructor
_TAG_CPP = {
    "pt": "point", "ls": "linestring", "poly": "polygon", "ring": "linearring",
    "mp": "multipoint", "mls": "multilinestring", "mpoly": "multipolygon",
}
_TAG_PY = {
    "pt": PyPoint, "ls": PyLineString, "poly": PyPolygon, "ring": PyLinearRing,
    "mp": PyMultiPoint, "mls": PyMultiLineString, "mpoly": PyMultiPolygon,
}
_FACTORY_NAMES = set(_TAG_CPP.values())

# Predicate ops: C++ suffix -> Python shapely method name
_PRED_OPS = {
    "contains": "contains", "within": "within", "disjoint": "disjoint",
    "touches": "touches", "crosses": "crosses", "overlaps": "overlaps",
    "covers": "covers", "covered_by": "covered_by",
    "equals": "equals", "equals_exact": "equals_exact",
    "intersects": "intersects", "relate": "relate",
    "hausdorff_distance": "hausdorff_distance",
}

# Regex patterns for API name dispatching
_PRED_RE       = re.compile(
    r'^(pt|ls|poly)_(' + '|'.join(_PRED_OPS.keys()) + r')_(pt|ls|poly)$')
_DIST_RE       = re.compile(r'^distance_(point|linestring|polygon)_(point|linestring|polygon)$')
_INTERSECTS_RE = re.compile(r'^intersects_(linestring|polygon)_(linestring|polygon)$')

_OPS_NAMES = {
    "centroid_point", "centroid_linestring", "centroid_polygon",
    "project_linestring_point", "interpolate_linestring",
    "intersection_area_polygon_polygon",
    "polygon_exterior",
    "nearest_points", "nearest_points_ls_pt",
}

# Geometry spec is a 2-tuple: (type_tag, coords).
# Type tags: pt, ls, poly, ring, mp, mls, mpoly
# Coords: (x,y) for pt; list of (x,y) for ls/poly/ring/mp;
#         list of lists for mls/mpoly


def _build_cpp_geom(cpp, gtype, coords):
    if gtype == "pt":
        return cpp.point(float(coords[0]), float(coords[1]))
    if gtype in ("ls", "poly", "ring", "mp"):
        arr = np.array(coords, dtype=np.float64)
        return getattr(cpp, _TAG_CPP[gtype])(arr)
    if gtype in ("mls", "mpoly"):
        arrays = [np.array(c, dtype=np.float64) for c in coords]
        return getattr(cpp, _TAG_CPP[gtype])(arrays)
    raise ValueError(f"Unknown geometry type: {gtype}")


def _build_py_geom(gtype, coords):
    cls = _TAG_PY[gtype]
    if gtype == "pt":
        return cls(float(coords[0]), float(coords[1]))
    return cls(coords)


def _parse_args(args):
    """Split args into [(gtype, coords), ...] geometry specs and [scalar, ...] extras."""
    geoms = []
    extras = []
    for a in args:
        if (isinstance(a, tuple) and len(a) == 2
                and isinstance(a[0], str) and a[0] in _TAG_CPP):
            geoms.append(a)
        else:
            extras.append(a)
    return geoms, extras


# ---- Main dispatch ----------------------------------------------------------

def call_cpp_py(api_name, cpp, *args, **kwargs):
    """Dispatch C++ and Python shapely calls based on api_name."""
    if api_name in _FACTORY_NAMES:
        return _call_factory(api_name, cpp, *args, **kwargs)
    m = _PRED_RE.match(api_name)
    if m:
        return _call_predicate(m, cpp, *args, **kwargs)
    m = _DIST_RE.match(api_name)
    if m:
        return _call_distance(m, cpp, *args, **kwargs)
    m = _INTERSECTS_RE.match(api_name)
    if m:
        return _call_intersects(m, cpp, *args, **kwargs)
    if api_name in _OPS_NAMES:
        return _call_ops(api_name, cpp, *args, **kwargs)
    if api_name.startswith("class."):
        return _call_class_method(api_name, cpp, *args, **kwargs)
    raise ValueError(f"Unknown API: {api_name}")


# ---- Factory dispatch -------------------------------------------------------

def _call_factory(api_name, cpp, *args, **kwargs):
    """Factory tests: return comparable arrays for 0-ULP comparison."""
    c_fn = getattr(cpp, api_name)

    if api_name == "point":
        geoms, _ = _parse_args(args)
        if geoms:
            x, y = float(geoms[0][1][0]), float(geoms[0][1][1])
        else:
            x, y = float(args[0]), float(args[1])
        c_pt = c_fn(x, y)
        py_pt = PyPoint(x, y)
        return np.array(c_pt.coords()), np.array(py_pt.coords)

    # Single-array factories: linestring, polygon, linearring, multipoint
    coords = args[0]
    if isinstance(coords, np.ndarray):
        arr = coords  # already a numpy array (e.g. (N,3) test)
    else:
        arr = np.array(coords, dtype=np.float64)

    if api_name in ("linestring", "polygon", "linearring"):
        c_geom = c_fn(arr)
        # Python shapely: if input is ndarray with >2 cols, strip to (N,2)
        if isinstance(coords, np.ndarray) and coords.ndim == 2 and coords.shape[1] > 2:
            py_coords_in = [(float(coords[i, 0]), float(coords[i, 1]))
                            for i in range(coords.shape[0])]
        else:
            py_coords_in = coords
        py_tag = {"linestring": "ls", "polygon": "poly", "linearring": "ring"}[api_name]
        py_geom = _TAG_PY[py_tag](py_coords_in)
        if api_name == "polygon":
            py_coords = np.array(list(py_geom.exterior.coords)[:-1])
        else:
            py_coords = np.array(py_geom.coords)
        return np.array(c_geom.coords()), py_coords

    # Multi-geometry factories: multipoint, multilinestring, multipolygon
    if api_name == "multipoint":
        c_geom = c_fn(arr)
        py_geom = PyMultiPoint(coords)
        return np.array([c_geom.area(), c_geom.length()]), \
               np.array([py_geom.area, py_geom.length])

    if api_name in ("multilinestring", "multipolygon"):
        coords_list = args[0]
        arrays = [np.array(c, dtype=np.float64) for c in coords_list]
        c_geom = c_fn(arrays)
        if api_name == "multilinestring":
            py_geom = PyMultiLineString([PyLineString(c) for c in coords_list])
        else:
            py_geom = PyMultiPolygon([PyPolygon(c) for c in coords_list])
        return np.array([c_geom.area(), c_geom.length()]), \
               np.array([py_geom.area, py_geom.length])

    raise ValueError(f"Unknown factory: {api_name}")


# ---- Predicate dispatch -----------------------------------------------------

def _call_predicate(m, cpp, *args, **kwargs):
    src, op, tgt = m.group(1), m.group(2), m.group(3)
    py_op = _PRED_OPS[op]
    geoms, extras = _parse_args(args)
    if len(geoms) < 2:
        raise ValueError(f"Predicate {src}_{op}_{tgt} needs 2 geoms: {args}")

    c_a = _build_cpp_geom(cpp, *geoms[0])
    c_b = _build_cpp_geom(cpp, *geoms[1])
    py_a = _build_py_geom(*geoms[0])
    py_b = _build_py_geom(*geoms[1])

    c_fn = getattr(cpp, f"{src}_{op}_{tgt}")
    if op == "equals_exact":
        tol = float(extras[0]) if extras else 1e-6
        return c_fn(c_a, c_b, tol), py_a.equals_exact(py_b, tol)
    return c_fn(c_a, c_b), getattr(py_a, py_op)(py_b)


# ---- Distance dispatch ------------------------------------------------------

def _call_distance(m, cpp, *args, **kwargs):
    src, tgt = m.group(1), m.group(2)
    geoms, _ = _parse_args(args)
    if len(geoms) < 2:
        raise ValueError(f"distance_{src}_{tgt} needs 2 geoms: {args}")

    c_a = _build_cpp_geom(cpp, *geoms[0])
    c_b = _build_cpp_geom(cpp, *geoms[1])
    py_a = _build_py_geom(*geoms[0])
    py_b = _build_py_geom(*geoms[1])

    c_fn = getattr(cpp, f"distance_{src}_{tgt}")
    return c_fn(c_a, c_b), py_a.distance(py_b)


# ---- Intersects dispatch ----------------------------------------------------

def _call_intersects(m, cpp, *args, **kwargs):
    src, tgt = m.group(1), m.group(2)
    geoms, _ = _parse_args(args)
    if len(geoms) < 2:
        raise ValueError(f"intersects_{src}_{tgt} needs 2 geoms: {args}")

    c_a = _build_cpp_geom(cpp, *geoms[0])
    c_b = _build_cpp_geom(cpp, *geoms[1])
    py_a = _build_py_geom(*geoms[0])
    py_b = _build_py_geom(*geoms[1])

    c_fn = getattr(cpp, f"intersects_{src}_{tgt}")
    return c_fn(c_a, c_b), py_a.intersects(py_b)


# ---- Named ops dispatch -----------------------------------------------------

def _call_ops(api_name, cpp, *args, **kwargs):
    geoms, extras = _parse_args(args)

    if api_name in ("centroid_point", "centroid_linestring", "centroid_polygon"):
        c_geom = _build_cpp_geom(cpp, *geoms[0])
        py_geom = _build_py_geom(*geoms[0])
        c_fn = getattr(cpp, api_name)
        c_result = c_fn(c_geom)
        py_centroid = py_geom.centroid
        py_result = (py_centroid.x, py_centroid.y)
        return np.array(c_result), np.array(py_result)

    if api_name == "project_linestring_point":
        c_ls = _build_cpp_geom(cpp, *geoms[0])
        c_pt = _build_cpp_geom(cpp, *geoms[1])
        py_ls = _build_py_geom(*geoms[0])
        py_pt = _build_py_geom(*geoms[1])
        return cpp.project_linestring_point(c_ls, c_pt), py_ls.project(py_pt)

    if api_name == "interpolate_linestring":
        c_ls = _build_cpp_geom(cpp, *geoms[0])
        py_ls = _build_py_geom(*geoms[0])
        dist = float(extras[0]) if extras else 5.0
        c_result = cpp.interpolate_linestring(c_ls, dist)
        py_pt = py_ls.interpolate(dist)
        return np.array(c_result), np.array((py_pt.x, py_pt.y))

    if api_name == "intersection_area_polygon_polygon":
        c_a = _build_cpp_geom(cpp, *geoms[0])
        c_b = _build_cpp_geom(cpp, *geoms[1])
        py_a = _build_py_geom(*geoms[0])
        py_b = _build_py_geom(*geoms[1])
        return (cpp.intersection_area_polygon_polygon(c_a, c_b),
                py_a.intersection(py_b).area)

    if api_name == "polygon_exterior":
        c_poly = _build_cpp_geom(cpp, *geoms[0])
        py_poly = _build_py_geom(*geoms[0])
        c_ext = cpp.polygon_exterior(c_poly)
        py_ext = np.array(py_poly.exterior.coords)
        # C++ exterior excludes closing point; Python includes it
        if py_ext.shape[0] > c_ext.shape[0]:
            py_ext = py_ext[:c_ext.shape[0]]
        return np.asarray(c_ext), py_ext

    if api_name == "nearest_points":
        c_a = _build_cpp_geom(cpp, *geoms[0])
        c_b = _build_cpp_geom(cpp, *geoms[1])
        py_a = _build_py_geom(*geoms[0])
        py_b = _build_py_geom(*geoms[1])
        r = py_ops.nearest_points(py_a, py_b)
        return (np.array(cpp.nearest_points(c_a, c_b)),
                np.array((r[0].x, r[0].y, r[1].x, r[1].y)))

    if api_name == "nearest_points_ls_pt":
        c_ls = _build_cpp_geom(cpp, *geoms[0])
        c_pt = _build_cpp_geom(cpp, *geoms[1])
        py_ls = _build_py_geom(*geoms[0])
        py_pt = _build_py_geom(*geoms[1])
        r = py_ops.nearest_points(py_ls, py_pt)
        return (np.array(cpp.nearest_points_ls_pt(c_ls, c_pt)),
                np.array((r[0].x, r[0].y, r[1].x, r[1].y)))

    raise ValueError(f"Unknown ops API: {api_name}")


# ---- Class method dispatch --------------------------------------------------

def _call_class_method(api_name, cpp, *args, **kwargs):
    """Test C++ geometry class methods against Python shapely equivalents.

    api_name format: "class.{geom_tag}.{method}"
    e.g. "class.pt.is_valid", "class.ls.distance", "class.poly.buffer"
    """
    parts = api_name.split(".")
    if len(parts) < 3:
        raise ValueError(f"Invalid class API: {api_name}")
    gtype, method = parts[1], parts[2]

    geoms, extras = _parse_args(args)
    if not geoms:
        raise ValueError(f"Class method '{api_name}' needs at least 1 geom spec")

    c_geom = _build_cpp_geom(cpp, *geoms[0])
    py_geom = _build_py_geom(*geoms[0])

    # -- Boolean properties (C++ method, Python property) --
    if method in ("has_z", "is_empty", "is_simple", "is_valid",
                  "is_closed", "is_ring", "is_ccw"):
        return getattr(c_geom, method)(), getattr(py_geom, method)

    # -- String properties --
    if method in ("wkt", "wkb_hex", "geom_type", "type"):
        return getattr(c_geom, method)(), getattr(py_geom, method)

    # -- Float properties --
    if method == "area":
        return c_geom.area(), py_geom.area
    if method == "length":
        return c_geom.length(), py_geom.length

    # -- Bounds --
    if method == "bounds":
        return np.array(c_geom.bounds()), np.array(py_geom.bounds)

    # -- Coords: C++ method, Python property (CoordinateSequence) --
    if method == "coords":
        c_result = np.array(c_geom.coords())
        if gtype == "poly":
            # C++ Polygon::coords() = exterior, no closing pt.
            # Python exterior.coords includes closing pt -> strip last row.
            py_result = np.array(list(py_geom.exterior.coords)[:-1])
        else:
            py_result = np.array(py_geom.coords)
        return c_result, py_result

    # -- xy: C++ returns (xs_list, ys_list) --
    if method == "xy":
        c_xs, c_ys = c_geom.xy()
        py_xs, py_ys = py_geom.xy
        return (list(c_xs), list(c_ys)), (list(py_xs), list(py_ys))

    # -- num_geometries (Multi*) --
    if method == "num_geometries":
        return c_geom.num_geometries(), len(py_geom.geoms)

    # -- geometry_n (Multi*) --
    if method == "geometry_n":
        idx = int(extras[0]) if extras else 0
        c_sub = c_geom.geometry_n(idx)
        py_sub = py_geom.geoms[idx]
        return c_sub.wkt(), py_sub.wkt

    # -- Distance (same-type, binary method) --
    if method == "distance":
        if len(geoms) < 2:
            raise ValueError(f"class.{gtype}.distance needs 2 geoms")
        c_b = _build_cpp_geom(cpp, *geoms[1])
        py_b = _build_py_geom(*geoms[1])
        return c_geom.distance(c_b), py_geom.distance(py_b)

    # -- Buffer (returns geometry; compare area) --
    if method == "buffer":
        dist = float(extras[0]) if extras else 1.0
        c_buf = c_geom.buffer(dist)
        py_buf = py_geom.buffer(dist)
        return c_buf.area(), py_buf.area

    # -- Intersects (same-type, binary method) --
    if method == "intersects":
        if len(geoms) < 2:
            raise ValueError(f"class.{gtype}.intersects needs 2 geoms")
        c_b = _build_cpp_geom(cpp, *geoms[1])
        py_b = _build_py_geom(*geoms[1])
        return c_geom.intersects(c_b), py_geom.intersects(py_b)

    raise ValueError(f"Unknown class method: {api_name}")


# =============================================================================
# F5: api_catalog() -- full API test case catalog (5 groups)
# =============================================================================

_TestCase = namedtuple('TestCase', [
    'api_name',     # C++ API name
    'args',         # positional args: geometry specs + scalars
    'kwargs',       # keyword args (reserved)
    'dtype_label',  # dtype for pytest ID
    'category',     # data category for pytest ID
    'cmp_strategy', # bit_exact | scalar_eq | bool_eq | str_eq
    'check_py',     # always True
    'setup_fn',     # reserved
    'group',        # factories | accessors | distance | predicates | ops
], defaults=("",))


# ---- Geometry data presets --------------------------------------------------

_SQ    = [(0, 0), (10, 0), (10, 10), (0, 10)]
_SQ2   = [(5, 0), (15, 0), (15, 10), (5, 10)]
_SQ3   = [(20, 0), (30, 0), (30, 10), (20, 10)]
_SQ_BIG= [(0, 0), (20, 0), (20, 20), (0, 20)]
_TRI   = [(0, 0), (10, 0), (5, 10)]
_L_H   = [(0, 5), (10, 5)]
_L_V   = [(5, 0), (5, 10)]
_L_D   = [(0, 0), (10, 10)]
_L_3   = [(0, 0), (5, 5), (10, 0)]
_L_OUT = [(20, 0), (20, 10)]
_R_SQ  = [(0, 0), (10, 0), (10, 10), (0, 0)]
_R_TRI = [(0, 0), (10, 0), (5, 10), (0, 0)]
_MPTS  = [(0, 0), (10, 10), (20, 5)]
_MLS   = [[(0, 0), (5, 0)], [(10, 0), (15, 0)]]
_MPOLY = [_SQ, _SQ3]
_I_A   = [(0, 0), (5, 0), (5, 5), (0, 5)]
_I_B   = [(3, 0), (8, 0), (8, 5), (3, 5)]


# ---- Group 1: factories (7 APIs) --------------------------------------------

def _catalog_factories():
    # Strategy: factories create geom objects -> verify via coords (bit_exact)
    # point -- scalar form
    for x, y, tag in [(0, 0, "origin"), (3.14, -2.71, "pi"),
                       (1e6, -1e-6, "large"), (-100.0, 200.0, "neg")]:
        yield _TestCase("point", (("pt", (x, y)),), {}, "f64", tag,
                       "bit_exact", True, None, "factories")

    # linestring
    for coords, tag in [(_L_H, "h"), (_L_V, "v"), (_L_D, "d"), (_L_3, "3pt")]:
        yield _TestCase("linestring", (coords,), {}, "f64", tag,
                       "bit_exact", True, None, "factories")
    # (N,3) array: z-column ignored check
    yield _TestCase("linestring",
                   (np.array([[0, 0, 99], [10, 0, 88], [10, 10, 77]],
                             dtype=np.float64),),
                   {}, "f64", "n3", "bit_exact", True, None, "factories")

    # polygon
    for coords, tag in [(_SQ, "sq"), (_TRI, "tri")]:
        yield _TestCase("polygon", (coords,), {}, "f64", tag,
                       "bit_exact", True, None, "factories")

    # linearring
    for coords, tag in [(_R_SQ, "sq"), (_R_TRI, "tri")]:
        yield _TestCase("linearring", (coords,), {}, "f64", tag,
                       "bit_exact", True, None, "factories")

    # multipoint
    yield _TestCase("multipoint", (_MPTS,), {}, "f64", "3pt",
                   "bit_exact", True, None, "factories")

    # multilinestring
    yield _TestCase("multilinestring", (_MLS,), {}, "f64", "2lines",
                   "bit_exact", True, None, "factories")

    # multipolygon
    yield _TestCase("multipolygon", (_MPOLY,), {}, "f64", "2sq",
                   "bit_exact", True, None, "factories")


# ---- Group 2: accessors -----------------------------------------------------

def _catalog_accessors():
    # Point accessors
    for x, y, tag in [(0, 0, "origin"), (3.14, -2.71, "pi"),
                       (1e6, -1e-6, "large")]:
        pt = ("pt", (x, y))
        for method, strat in [
            ("area", "scalar_eq"), ("length", "scalar_eq"),
            ("bounds", "bit_exact"), ("coords", "bit_exact"),
            ("is_empty", "bool_eq"), ("is_simple", "bool_eq"),
            ("is_valid", "bool_eq"), ("has_z", "bool_eq"),
        ]:
            yield _TestCase(f"class.pt.{method}", (pt,), {}, "f64", tag,
                           strat, True, None, "accessors")
        # WKT/WKB: compare strings strictly
        yield _TestCase(f"class.pt.wkt", (pt,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.pt.wkb_hex", (pt,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.pt.geom_type", (pt,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.pt.type", (pt,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")

    # LineString accessors
    for coords, tag in [(_L_H, "h"), (_L_V, "v"), (_L_3, "3pt")]:
        ls = ("ls", coords)
        for method, strat in [
            ("area", "scalar_eq"), ("length", "scalar_eq"),
            ("bounds", "bit_exact"), ("coords", "bit_exact"),
            ("is_empty", "bool_eq"), ("is_simple", "bool_eq"),
            ("is_valid", "bool_eq"), ("has_z", "bool_eq"),
            ("is_closed", "bool_eq"), ("is_ring", "bool_eq"),
        ]:
            yield _TestCase(f"class.ls.{method}", (ls,), {}, "f64", tag,
                           strat, True, None, "accessors")
        yield _TestCase(f"class.ls.wkt", (ls,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.ls.wkb_hex", (ls,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.ls.geom_type", (ls,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.ls.type", (ls,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")

    # Polygon accessors
    for coords, tag in [(_SQ, "sq"), (_TRI, "tri")]:
        poly = ("poly", coords)
        for method, strat in [
            ("area", "scalar_eq"), ("length", "scalar_eq"),
            ("bounds", "bit_exact"), ("coords", "bit_exact"),
            ("is_empty", "bool_eq"), ("is_simple", "bool_eq"),
            ("is_valid", "bool_eq"), ("has_z", "bool_eq"),
        ]:
            yield _TestCase(f"class.poly.{method}", (poly,), {}, "f64", tag,
                           strat, True, None, "accessors")
        yield _TestCase(f"class.poly.wkt", (poly,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.poly.wkb_hex", (poly,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.poly.geom_type", (poly,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.poly.type", (poly,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")

    # LinearRing accessors
    for coords, tag in [(_R_SQ, "sq"), (_R_TRI, "tri")]:
        ring = ("ring", coords)
        for method, strat in [
            ("area", "scalar_eq"), ("length", "scalar_eq"),
            ("bounds", "bit_exact"), ("coords", "bit_exact"),
            ("is_empty", "bool_eq"), ("is_simple", "bool_eq"),
            ("is_valid", "bool_eq"), ("has_z", "bool_eq"),
            ("is_closed", "bool_eq"), ("is_ring", "bool_eq"),
            ("is_ccw", "bool_eq"),
        ]:
            yield _TestCase(f"class.ring.{method}", (ring,), {}, "f64", tag,
                           strat, True, None, "accessors")
        yield _TestCase(f"class.ring.wkt", (ring,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.ring.wkb_hex", (ring,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.ring.geom_type", (ring,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")
        yield _TestCase(f"class.ring.type", (ring,), {}, "f64", tag,
                       "str_eq", True, None, "accessors")


# ---- Group 3: distance ------------------------------------------------------

def _catalog_distance():
    # Same-type: Point.distance(Point)
    for p1, p2, tag in [
        (("pt", (0, 0)), ("pt", (3, 4)), "3-4-5"),
        (("pt", (0, 0)), ("pt", (0, 0)), "zero"),
        (("pt", (-1, -1)), ("pt", (2, 3)), "5.0"),
    ]:
        yield _TestCase("class.pt.distance", (p1, p2), {}, "f64", tag,
                       "scalar_eq", True, None, "distance")

    # Same-type: LineString.distance(LineString)
    for l1, l2, tag in [
        (("ls", _L_H), ("ls", [(0, 10), (10, 10)]), "parallel"),
        (("ls", _L_D), ("ls", [(0, 10), (10, 0)]), "crossing"),
    ]:
        yield _TestCase("class.ls.distance", (l1, l2), {}, "f64", tag,
                       "scalar_eq", True, None, "distance")

    # Same-type: Polygon.distance(Polygon)
    for p1, p2, tag in [
        (("poly", _SQ), ("poly", _SQ3), "disjoint"),
        (("poly", _SQ), ("poly", _SQ2), "overlap"),
    ]:
        yield _TestCase("class.poly.distance", (p1, p2), {}, "f64", tag,
                       "scalar_eq", True, None, "distance")

    # Cross-type: distance_point_linestring
    for pt, ls, tag in [
        (("pt", (3, 0)), ("ls", [(0, 0), (0, 10)]), "near_v"),
        (("pt", (0, 0)), ("ls", [(3, 0), (3, 10)]), "disjoint"),
        (("pt", (5, 0)), ("ls", [(0, 0), (10, 0)]), "on_line"),
    ]:
        yield _TestCase("distance_point_linestring", (pt, ls), {}, "f64", tag,
                       "scalar_eq", True, None, "distance")

    # Cross-type: distance_point_polygon
    for pt, poly, tag in [
        (("pt", (5, 5)), ("poly", _SQ), "inside"),
        (("pt", (20, 0)), ("poly", _SQ), "outside"),
        (("pt", (0, 5)), ("poly", _SQ), "on_edge"),
    ]:
        yield _TestCase("distance_point_polygon", (pt, poly), {}, "f64", tag,
                       "scalar_eq", True, None, "distance")

    # Cross-type: distance_linestring_polygon + distance_polygon_linestring
    for name, g1, g2, tag in [
        ("distance_linestring_polygon", ("ls", [(10, 0), (10, 10)]),
         ("poly", _SQ), "touch"),
        ("distance_linestring_polygon", ("ls", _L_OUT),
         ("poly", _SQ), "outside"),
        ("distance_linestring_polygon", ("ls", [(-10, 0), (10, 0)]),
         ("poly", _SQ), "cross"),
        ("distance_polygon_linestring", ("poly", _SQ),
         ("ls", [(10, 0), (10, 10)]), "touch_rev"),
    ]:
        yield _TestCase(name, (g1, g2), {}, "f64", tag,
                       "scalar_eq", True, None, "distance")


# ---- Group 4: predicates ----------------------------------------------------

def _catalog_predicates():
    # -- pt <-> pt (representative subset) --
    for op, p1, p2, strat, tag in [
        ("contains",    ("pt", (0, 0)), ("pt", (0, 0)), "bool_eq", "same"),
        ("within",      ("pt", (0, 0)), ("pt", (1, 1)), "bool_eq", "diff"),
        ("disjoint",    ("pt", (0, 0)), ("pt", (0, 0)), "bool_eq", "same"),
        ("touches",     ("pt", (0, 0)), ("pt", (0, 0)), "bool_eq", "same"),
        ("intersects",  ("pt", (0, 0)), ("pt", (1, 1)), "bool_eq", "diff"),
        ("equals",      ("pt", (0, 0)), ("pt", (0, 0)), "bool_eq", "same"),
        ("covers",      ("pt", (0, 0)), ("pt", (0, 0)), "bool_eq", "same"),
        ("crosses",     ("pt", (0, 0)), ("pt", (1, 1)), "bool_eq", "diff"),
        ("overlaps",    ("pt", (0, 0)), ("pt", (1, 1)), "bool_eq", "diff"),
        ("hausdorff_distance", ("pt", (0, 0)), ("pt", (3, 4)),
         "scalar_eq", "3-4-5"),
    ]:
        extra = (1e-6,) if op == "equals_exact" else ()
        yield _TestCase(f"pt_{op}_pt", (p1, p2) + extra, {}, "f64", tag,
                       strat, True, None, "predicates")

    # -- pt <-> poly (collision-critical) --
    for op, pt, poly, tag in [
        ("within",    ("pt", (5, 5)), ("poly", _SQ), "inside"),
        ("disjoint",  ("pt", (20, 20)), ("poly", _SQ), "outside"),
        ("touches",   ("pt", (0, 5)), ("poly", _SQ), "edge"),
        ("intersects",("pt", (5, 5)), ("poly", _SQ), "inside"),
        ("intersects",("pt", (20, 20)), ("poly", _SQ), "outside"),
        ("contains",  ("pt", (5, 5)), ("poly", _SQ), "inside_pt"),
        ("covers",    ("pt", (5, 5)), ("poly", _TRI), "inside_tri"),
    ]:
        yield _TestCase(f"pt_{op}_poly", (pt, poly), {}, "f64", tag,
                       "bool_eq", True, None, "predicates")

    # -- pt <-> ls --
    for op, pt, ls, tag in [
        ("within", ("pt", (5, 5)), ("ls", _L_H), "on"),
        ("disjoint", ("pt", (5, 10)), ("ls", _L_H), "off"),
        ("intersects", ("pt", (5, 5)), ("ls", _L_H), "on"),
    ]:
        yield _TestCase(f"pt_{op}_ls", (pt, ls), {}, "f64", tag,
                       "bool_eq", True, None, "predicates")

    # -- ls <-> pt --
    for op, ls, pt, tag in [
        ("contains", ("ls", _L_H), ("pt", (5, 5)), "on"),
        ("intersects", ("ls", _L_H), ("pt", (5, 5)), "on"),
        ("disjoint", ("ls", _L_H), ("pt", (5, 10)), "off"),
    ]:
        yield _TestCase(f"ls_{op}_pt", (ls, pt), {}, "f64", tag,
                       "bool_eq", True, None, "predicates")

    # -- ls <-> ls --
    for op, ls1, ls2, tag in [
        ("crosses",  ("ls", _L_H), ("ls", _L_V), "cross"),
        ("disjoint", ("ls", _L_H), ("ls", [(0, 10), (10, 10)]), "parallel"),
        ("intersects", ("ls", _L_H), ("ls", _L_V), "cross"),
    ]:
        yield _TestCase(f"ls_{op}_ls", (ls1, ls2), {}, "f64", tag,
                       "bool_eq", True, None, "predicates")

    # -- ls <-> poly --
    for op, ls, poly, tag in [
        ("intersects", ("ls", _L_H), ("poly", _SQ), "cross"),
        ("intersects", ("ls", _L_OUT), ("poly", _SQ), "outside"),
        ("disjoint", ("ls", _L_OUT), ("poly", _SQ), "outside"),
        ("crosses", ("ls", _L_H), ("poly", _SQ), "cross"),
    ]:
        yield _TestCase(f"ls_{op}_poly", (ls, poly), {}, "f64", tag,
                       "bool_eq", True, None, "predicates")

    # -- poly <-> pt --
    for op, poly, pt, tag in [
        ("contains", ("poly", _SQ), ("pt", (5, 5)), "inside"),
        ("covers", ("poly", _SQ), ("pt", (0, 0)), "vertex"),
        ("intersects", ("poly", _SQ), ("pt", (5, 5)), "inside"),
        ("disjoint", ("poly", _SQ), ("pt", (20, 20)), "outside"),
    ]:
        yield _TestCase(f"poly_{op}_pt", (poly, pt), {}, "f64", tag,
                       "bool_eq", True, None, "predicates")

    # -- poly <-> ls --
    for op, poly, ls, tag in [
        ("intersects", ("poly", _SQ), ("ls", _L_H), "cross"),
        ("crosses", ("poly", _SQ), ("ls", _L_H), "cross"),
        ("disjoint", ("poly", _SQ), ("ls", _L_OUT), "outside"),
    ]:
        yield _TestCase(f"poly_{op}_ls", (poly, ls), {}, "f64", tag,
                       "bool_eq", True, None, "predicates")

    # -- poly <-> poly (collision-critical) --
    for op, p1, p2, strat, tag in [
        ("overlaps", ("poly", _SQ), ("poly", _SQ2), "bool_eq", "overlap"),
        ("disjoint", ("poly", _SQ), ("poly", _SQ3), "bool_eq", "disjoint"),
        ("touches", ("poly", _SQ), ("poly", _SQ2), "bool_eq", "overlap"),
        ("intersects", ("poly", _SQ), ("poly", _SQ2), "bool_eq", "overlap"),
        ("intersects", ("poly", _SQ), ("poly", _SQ3), "bool_eq", "disjoint"),
        ("contains", ("poly", _SQ_BIG), ("poly", _SQ), "bool_eq", "contains"),
        ("covers", ("poly", _SQ), ("poly", _SQ), "bool_eq", "equal"),
    ]:
        yield _TestCase(f"poly_{op}_poly", (p1, p2), {}, "f64", tag,
                       strat, True, None, "predicates")


# ---- Group 5: ops -----------------------------------------------------------

def _catalog_ops():
    # -- Buffer (compare area) --
    for gtype, coords, dist, tag in [
        ("pt", (0, 0), 1.0, "pt_r1"),
        ("pt", (0, 0), 5.0, "pt_r5"),
        ("pt", (0, 0), 0.0, "pt_r0"),
        ("ls", _L_H, 1.0, "ls_r1"),
        ("ls", _L_H, 0.0, "ls_r0"),
        ("poly", _SQ, 2.0, "poly_r2"),
        ("poly", _SQ, 0.0, "poly_r0"),
    ]:
        gtag = gtype if gtype != "poly" else "poly"
        yield _TestCase(f"class.{gtype}.buffer", ((gtag, coords), dist), {},
                       "f64", tag, "scalar_eq", True, None, "ops")

    # -- Cross-type intersects (free functions) --
    for name, g1, g2, tag in [
        ("intersects_polygon_polygon", ("poly", _SQ),
         ("poly", _SQ2), "overlap"),
        ("intersects_polygon_polygon", ("poly", _SQ),
         ("poly", _SQ3), "disjoint"),
        ("intersects_polygon_polygon", ("poly", _SQ_BIG),
         ("poly", _SQ), "contains"),
        ("intersects_linestring_polygon", ("ls", _L_H),
         ("poly", _SQ), "cross"),
        ("intersects_linestring_polygon", ("ls", _L_OUT),
         ("poly", _SQ), "outside"),
        ("intersects_polygon_linestring", ("poly", _SQ),
         ("ls", _L_H), "cross_rev"),
    ]:
        yield _TestCase(name, (g1, g2), {}, "f64", tag,
                       "bool_eq", True, None, "ops")

    # -- project_linestring_point --
    for ls, pt, tag in [
        (("ls", _L_H), ("pt", (5, 3)), "h_mid_above"),
        (("ls", _L_V), ("pt", (3, 5)), "v_mid_right"),
        (("ls", _L_H), ("pt", (0, 0)), "h_start"),
        (("ls", _L_H), ("pt", (10, 0)), "h_end"),
    ]:
        yield _TestCase("project_linestring_point", (ls, pt), {}, "f64", tag,
                       "scalar_eq", True, None, "ops")

    # -- interpolate_linestring --
    for ls, dist, tag in [
        (("ls", _L_H), 5.0, "h_mid"),
        (("ls", _L_V), 5.0, "v_mid"),
        (("ls", _L_H), 0.0, "h_start"),
        (("ls", _L_H), 10.0, "h_end"),
        (("ls", _L_3), 10.0, "v3pt"),
    ]:
        yield _TestCase("interpolate_linestring", (ls, dist), {}, "f64", tag,
                       "bit_exact", True, None, "ops")

    # -- Centroid --
    for name, geom, tag in [
        ("centroid_point", ("pt", (3, 4)), "3-4"),
        ("centroid_point", ("pt", (-10, 20)), "neg"),
        ("centroid_linestring", ("ls", _L_D), "d"),
        ("centroid_linestring", ("ls", _L_3), "v"),
        ("centroid_polygon", ("poly", _SQ), "sq"),
        ("centroid_polygon", ("poly", _TRI), "tri"),
    ]:
        yield _TestCase(name, (geom,), {}, "f64", tag,
                       "bit_exact", True, None, "ops")

    # -- intersection_area_polygon_polygon --
    for p1, p2, tag in [
        (("poly", _I_A), ("poly", _I_B), "overlap"),
        (("poly", _SQ), ("poly", _SQ3), "disjoint"),
    ]:
        yield _TestCase("intersection_area_polygon_polygon", (p1, p2), {},
                       "f64", tag, "scalar_eq", True, None, "ops")

    # -- polygon_exterior --
    for coords, tag in [(_SQ, "sq"), (_TRI, "tri")]:
        yield _TestCase("polygon_exterior", (("poly", coords),), {}, "f64", tag,
                       "bit_exact", True, None, "ops")

    # -- nearest_points --
    for name, g1, g2, tag in [
        ("nearest_points", ("poly", _SQ),
         ("ls", [(10, 0), (15, 0)]), "poly_ls_touch"),
        ("nearest_points", ("poly", _SQ),
         ("ls", [(5.1, 0), (5.1, 10)]), "poly_ls_near"),
        ("nearest_points_ls_pt", ("ls", _L_H),
         ("pt", (5, 3)), "ls_pt_above"),
        ("nearest_points_ls_pt", ("ls", _L_H),
         ("pt", (15, 0)), "ls_pt_endpoint"),
    ]:
        yield _TestCase(name, (g1, g2), {}, "f64", tag,
                       "bit_exact", True, None, "ops")


# =============================================================================
# api_catalog() -- summary entry
# =============================================================================

def api_catalog():
    """Export all shapelycpp test cases, organized by group."""
    for tc in _catalog_factories():
        yield tc._replace(group="factories")
    for tc in _catalog_accessors():
        yield tc._replace(group="accessors")
    for tc in _catalog_distance():
        yield tc._replace(group="distance")
    for tc in _catalog_predicates():
        yield tc._replace(group="predicates")
    for tc in _catalog_ops():
        yield tc._replace(group="ops")


# =============================================================================
# Module loading (identical pattern to numpypcpp)
# =============================================================================

_cpp_module = None
_import_error = None


def _resolve_module_name():
    return (getattr(pytest, "_shapelycpp_module_name", None)
            or os.environ.get("SHAPELYCPP_MODULE")
            or "shapelycpp")


def get_cpp_module():
    global _cpp_module, _import_error
    if _cpp_module is not None:
        return _cpp_module
    if _import_error is not None:
        raise _import_error
    try:
        _cpp_module = importlib.import_module(_resolve_module_name())
    except ImportError as e:
        _import_error = e
        raise
    return _cpp_module


@pytest.fixture(scope="session")
def cpp():
    return get_cpp_module()


# =============================================================================
# build_all_tests -- catalog -> pytest.param
# =============================================================================

def build_all_tests():
    """Expand api_catalog() into pytest parametrized list."""
    for tc in api_catalog():
        test_id = f"{tc.api_name}[{tc.dtype_label}][{tc.category}]"
        yield pytest.param(tc, id=test_id)


# =============================================================================
# Parametrized test function + report.csv collection
# =============================================================================

_REPORT_ROWS = []


def _geos_info(cpp):
    try:
        return getattr(cpp, "geos_version", lambda: "unknown")()
    except Exception:
        return "unknown"


@pytest.mark.parametrize("tc", list(build_all_tests()))
def test_api(tc, cpp):
    """Full test pipeline: F5(catalog)->F4(dispatch)->F3(compare) + report."""
    api_name  = tc.api_name
    args      = tc.args
    kwargs    = tc.kwargs
    strategy  = tc.cmp_strategy
    check_py  = tc.check_py

    if tc.setup_fn is not None:
        tc.setup_fn(args, kwargs)
        return

    cpp_r, py_r = call_cpp_py(api_name, cpp, *args, **kwargs)
    if check_py and py_r is None:
        pytest.skip(f"no shapely equivalent for {api_name}")

    try:
        compare(cpp_r, py_r, strategy=strategy, label=f"{api_name}")
        _REPORT_ROWS.append(dict(
            category=tc.group, api=api_name, mode=_geos_info(cpp),
            dtype=tc.dtype_label, feature=tc.category,
            result="PASS", ulp=0))
    except AssertionError:
        _REPORT_ROWS.append(dict(
            category=tc.group, api=api_name, mode=_geos_info(cpp),
            dtype=tc.dtype_label, feature=tc.category,
            result="FAIL", ulp=-1))
        raise


@pytest.fixture(scope="session", autouse=True)
def _write_report_csv():
    """Write report.csv after all tests complete."""
    yield
    import csv
    fname = os.environ.get("REPORT_CSV", "report.csv")
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), fname)
    if not _REPORT_ROWS:
        return
    with open(path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["category", "api", "mode", "dtype",
                                          "feature", "result", "ulp"])
        w.writeheader()
        w.writerows(_REPORT_ROWS)
    print(f"\nreport.csv: {len(_REPORT_ROWS)} rows -> {path}")


# =============================================================================
# __main__
# =============================================================================

if __name__ == "__main__":
    import sys
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    sys.exit(pytest.main([__file__, "-q", "--tb=short", "--no-header"]))
