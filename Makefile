# Makefile
CXX 				:= g++
CXXFLAGS 		:= -std=c++2b -Wall -Wextra -I. -I./lib/doctest

.PHONY: submodule test clean ultra-clean

submodule:
	git submodule update --init --recursive

test: submodule
	mkdir -p ./build
	$(CXX) $(CXXFLAGS) -o ./build/test ./test/test.cpp
	./build/test

clean:
	rm -rf ./build

ultra-clean: clean
	rm -rf ./lib

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -c $< -o $@
