all: build
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make

build:
	mkdir build
