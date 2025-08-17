# Makefile
CXX 				:= g++
CXXFLAGS 		:= -std=c++2b -Wall -Wextra -I. -I./lib/doctest

.PHONY: submodule test clean ultra-clean

all: test test_as_result_status

submodule:
	git submodule update --init --recursive

test_as_result_status: submodule
	mkdir -p ./build
	$(CXX) $(CXXFLAGS) -o ./build/test_as_result_status ./test/test_as_result_status.cpp
	./build/test_as_result_status


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
