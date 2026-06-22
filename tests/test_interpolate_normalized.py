"""Test: shapelycpp interpolate_ls(normalized=True) vs Python shapely."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))

import numpy as np
from shapely.geometry import LineString as PyLineString
from test_api import cpp  # reuse existing test fixture

def test_normalized_random():
    """100 random lines, 11 normalized positions each."""
    np.random.seed(42)
    failures = []
    for _ in range(100):
        n = np.random.randint(2, 10)
        pts = np.random.randn(n, 2) * 50
        pts[:, 0] += np.arange(n) * 10
        line = pts.tolist()
        cpp_ls = cpp.linestring(line)
        py_ls = PyLineString(line)
        for i in range(11):
            t = float(i) / 10.0
            x, y = cpp.interpolate_linestring(cpp_ls, t, True)
            py_pt = py_ls.interpolate(t, normalized=True)
            dx = abs(x - py_pt.x)
            dy = abs(y - py_pt.y)
            if dx > 0 or dy > 0:
                failures.append((line, t, (x, y), (py_pt.x, py_pt.y), dx, dy))
    assert len(failures) == 0, f"{len(failures)} failures: {failures[:3]}"
    print(f"PASSED: 100 random lines × 11 positions = 0 mismatch")

if __name__ == "__main__":
    test_normalized_random()
