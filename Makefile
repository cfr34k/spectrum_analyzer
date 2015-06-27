# Small Makefile wrapper to make it easier to use the cmake build system with
# vim.

all: build-debug

build-debug:
	@if [ ! -x build/debug ]; then echo "You must run setup.sh first!"; exit 1; fi
	(cd build/debug; $(MAKE) )

build-release:
	@if [ ! -x build/release ]; then echo "You must run setup.sh first!"; exit 1; fi
	(cd build/release; $(MAKE) )

debug: build-debug
	cgdb build/debug/floofs-core

doc: $(INCLUDES)
	doxygen Doxyfile

view-doc: doc
	xdg-open doc/html/index.html


