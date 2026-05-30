"""
Pytest fixtures and hooks for shapelycpp precision alignment tests.
"""

import numpy as np
import pytest


def pytest_addoption(parser):
    parser.addoption("--cpp-module", action="store", default=None,
        help="Python module name for the compiled C++ shapelycpp library")
    parser.addoption("--rtol", action="store", type=float, default=1e-10)
    parser.addoption("--atol", action="store", type=float, default=1e-10)

# -- Lazy C++ module import ---------------------------------------------------

_cpp_module = None
_import_error = None

def _resolve_module_name(config):
    import os
    cli = config.getoption("--cpp-module", default=None)
    if cli: return cli
    env = os.environ.get("SHAPELYCPP_MODULE")
    if env: return env
    return "shapelycpp"

def get_cpp_module(request=None):
    global _cpp_module, _import_error
    if _cpp_module is not None: return _cpp_module
    if _import_error is not None: raise _import_error
    if request is not None:
        modname = _resolve_module_name(request.config)
    else:
        import os
        modname = os.environ.get("SHAPELYCPP_MODULE", "shapelycpp")
    import importlib
    try:
        _cpp_module = importlib.import_module(modname)
    except ImportError as e:
        _import_error = e
        raise
    return _cpp_module

# -- Fixtures ------------------------------------------------------------------

@pytest.fixture(scope="session")
def cpp():
    return get_cpp_module()

@pytest.fixture
def rtol(request):
    return request.config.getoption("--rtol", default=1e-10)

@pytest.fixture
def atol(request):
    return request.config.getoption("--atol", default=1e-10)

@pytest.fixture(params=[np.float64, np.float32], ids=["float64", "float32"])
def dtype(request):
    return request.param

@pytest.fixture(params=[np.float64, np.float32], ids=["float64", "float32"])
def make(cpp, request):
    """Parametrized fixture providing float64/float32 geometry factories and classes.

    All factories use the SAME Python name (e.g. cpp.point) — dtype is selected
    by passing numpy arrays with the appropriate dtype.  pybind11 overload
    resolution picks the matching C++ template instantiation."""
    if request.param == np.float64:
        return {
            'point': lambda x, y: cpp.point(float(x), float(y)),
            'linestring': lambda coords: cpp.linestring(np.array(coords, dtype=np.float64)),
            'polygon': lambda coords: cpp.polygon(np.array(coords, dtype=np.float64)),
            'multipoint': lambda coords: cpp.multipoint(np.array(coords, dtype=np.float64)),
            'multilinestring': lambda lines: cpp.multilinestring([np.array(l, dtype=np.float64) for l in lines]),
            'multipolygon': lambda polys: cpp.multipolygon([np.array(p, dtype=np.float64) for p in polys]),
            'Point': cpp.Point, 'LineString': cpp.LineString, 'Polygon': cpp.Polygon,
            'MultiPoint': cpp.MultiPoint, 'MultiLineString': cpp.MultiLineString, 'MultiPolygon': cpp.MultiPolygon,
            'dtype': np.float64,
        }
    else:
        return {
            'point': lambda x, y: cpp.point(np.array([x, y], dtype=np.float32)),
            'linestring': lambda coords: cpp.linestring(np.array(coords, dtype=np.float32)),
            'polygon': lambda coords: cpp.polygon(np.array(coords, dtype=np.float32)),
            'multipoint': lambda coords: cpp.multipoint(np.array(coords, dtype=np.float32)),
            'multilinestring': lambda lines: cpp.multilinestring([np.array(l, dtype=np.float32) for l in lines]),
            'multipolygon': lambda polys: cpp.multipolygon([np.array(p, dtype=np.float32) for p in polys]),
            'Point': cpp.PointF32, 'LineString': cpp.LineStringF32, 'Polygon': cpp.PolygonF32,
            'MultiPoint': cpp.MultiPoint, 'MultiLineString': cpp.MultiLineString, 'MultiPolygon': cpp.MultiPolygon,
            'dtype': np.float32,
        }

# -- CppFactory convenience wrapper -------------------------------------------

class CppFactory:
    def __init__(self, cpp_mod):
        self.m = cpp_mod
    def point(self, x, y, dtype='double'):
        if dtype == 'float32':
            return self.m.point(np.array([float(x), float(y)], dtype=np.float32))
        return self.m.point(float(x), float(y))
    def linestring(self, coords, dtype='double'):
        if dtype == 'float32':
            return self.m.linestring(np.array(coords, dtype=np.float32))
        return self.m.linestring(np.array(coords, dtype=np.float64))
    def polygon(self, coords, dtype='double'):
        if dtype == 'float32':
            return self.m.polygon(np.array(coords, dtype=np.float32))
        return self.m.polygon(np.array(coords, dtype=np.float64))
    def linearring(self, coords):
        return self.m.linearring(np.array(coords, dtype=np.float64))
    def multipoint(self, coords, dtype='double'):
        if dtype == 'float32':
            return self.m.multipoint(np.array(coords, dtype=np.float32))
        return self.m.multipoint(np.array(coords, dtype=np.float64))
    def multilinestring(self, lines, dtype='double'):
        if dtype == 'float32':
            return self.m.multilinestring([np.array(l, dtype=np.float32) for l in lines])
        return self.m.multilinestring([np.array(l, dtype=np.float64) for l in lines])
    def multipolygon(self, polys, dtype='double'):
        if dtype == 'float32':
            return self.m.multipolygon([np.array(p, dtype=np.float32) for p in polys])
        return self.m.multipolygon([np.array(p, dtype=np.float64) for p in polys])

@pytest.fixture(scope='session')
def C(cpp):
    return CppFactory(cpp)
