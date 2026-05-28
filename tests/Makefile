# Build shapelycpp Python module for testing.
# Requires: pybind11, GEOS, Python3 development headers.

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -fPIC
INCLUDES  = -I.. $(shell python3 -m pybind11 --includes) $(shell /usr/local/bin/geos-config --cflags 2>/dev/null || echo -I/usr/local/include)
LDFLAGS   = -shared -L/usr/local/lib -lgeos
EXT_SUFFIX := $(shell python3-config --extension-suffix 2>/dev/null || echo .so)

MODULE    = shapelycpp$(EXT_SUFFIX)
SRC       = module.cpp

.PHONY: all clean test

all: $(MODULE)

$(MODULE): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS)

test: $(MODULE)
	cd .. && PYTHONPATH=tests:$$PYTHONPATH python3 -m pytest tests/ -v

clean:
	rm -f $(MODULE) shapelycpp*.so
