"""
Shared test utilities: comparison engine, data generators for shapelycpp tests.
"""
import numpy as np
from shapely.geometry import Point as PyPoint, LineString as PyLineString, Polygon as PyPolygon


def compare(cpp_result, py_result, rtol=1e-12, atol=1e-12, label=""):
    """Compare C++ result against Python shapely (ground-truth) result.

    Returns a dict with: pass, max_abs_diff, max_rel_diff, shape_match,
    cpp_shape, py_shape, cpp_dtype, py_dtype, label.
    """
    cpp = np.asarray(cpp_result, dtype=np.float64)
    py = np.asarray(py_result, dtype=np.float64)

    info = {
        "label": label,
        "shape_match": cpp.shape == py.shape,
        "cpp_shape": cpp.shape,
        "py_shape": py.shape,
        "cpp_dtype": str(cpp.dtype),
        "py_dtype": str(py.dtype),
    }

    if not info["shape_match"]:
        info["pass"] = False
        info["max_abs_diff"] = float("nan")
        info["max_rel_diff"] = float("nan")
        info["error"] = f"shape mismatch: C++ {cpp.shape} vs Python {py.shape}"
        return info

    abs_diff = np.abs(cpp - py)
    max_abs = float(np.max(abs_diff))

    py_abs = np.abs(py)
    with np.errstate(divide="ignore", invalid="ignore"):
        rel_diff = np.where(py_abs > 0, abs_diff / py_abs, abs_diff)
    max_rel = float(np.max(rel_diff))

    passed = bool(np.allclose(cpp, py, rtol=rtol, atol=atol))

    info["pass"] = passed
    info["max_abs_diff"] = max_abs
    info["max_rel_diff"] = max_rel

    if not passed:
        worst_idx = int(np.argmax(abs_diff))
        info["error"] = (
            f"numerical mismatch: max_abs_diff={max_abs:.2e}, "
            f"max_rel_diff={max_rel:.2e} at idx {worst_idx}\n"
            f"  C++ value: {cpp.flat[worst_idx]:.16e}\n"
            f"  Py  value: {py.flat[worst_idx]:.16e}"
        )

    return info


def assert_match(cpp_result, py_result, rtol=1e-12, atol=1e-12, label=""):
    """Like compare() but raises AssertionError on mismatch."""
    info = compare(cpp_result, py_result, rtol=rtol, atol=atol, label=label)
    if not info["pass"]:
        raise AssertionError(info.get("error", "mismatch"))
    return info


def tolerance_for(dtype, rtol, atol):
    """Return (rtol, atol) appropriate for the given dtype."""
    if dtype == np.float32:
        return max(rtol, 1e-6), max(atol, 1e-6)
    return rtol, atol


# ---- Geometry data generators -----------------------------------------------

def make_point_coords(seed=42):
    """Generate test point coordinates."""
    rng = np.random.RandomState(seed)
    return float(rng.uniform(-100, 100)), float(rng.uniform(-100, 100))


def make_linestring_coords(n=5, seed=42):
    """Generate a simple LineString as list of (x, y) tuples."""
    rng = np.random.RandomState(seed)
    return [(float(rng.uniform(-100, 100)), float(rng.uniform(-100, 100)))
            for _ in range(n)]


def make_square_coords(cx=0.0, cy=0.0, half=5.0):
    """Generate a square polygon (exterior ring) as list of (x,y)."""
    return [
        (cx - half, cy - half),
        (cx + half, cy - half),
        (cx + half, cy + half),
        (cx - half, cy + half),
    ]


def make_triangle_coords():
    """Generate a triangle polygon."""
    return [(0.0, 0.0), (10.0, 0.0), (5.0, 10.0)]


def py_point(x=0.0, y=0.0):
    return PyPoint(x, y)


def py_linestring(coords=None):
    if coords is None:
        coords = [(0, 0), (1, 1), (2, 2)]
    return PyLineString(coords)


def py_polygon(coords=None):
    if coords is None:
        coords = make_square_coords()
    return PyPolygon(coords)
