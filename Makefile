all: build
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make

build:
	mkdir build

test:
	nosetests

clean:
	rm -rf build
	rm -f *.str *.json
