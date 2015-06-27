#!/bin/sh

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

(
	mkdir -p build/debug
	cd build/debug

	cmake -DCMAKE_BUILD_TYPE=Debug ../..
)

(
	mkdir -p build/release
	cd build/release

	cmake -DCMAKE_BUILD_TYPE=Release ../..
)
