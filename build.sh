#!/bin/bash

build_dir="build"
cmakelists="CMakeLists.txt"

# getting project name from cmakelists.txt
PROJECT_NAME=$(grep -m 1 "^project" "$cmakelists" | sed 's/.*(\([^)]*\))/\1/')


# function to build the project
function build_process() {
	cd build/ || exit 1 # change build dir or exit on failuer
	echo "Starting build process..."
	cmake .. # run cmake
	echo "building $PROJECT_NAME"
	cmake --build . # build project 
	cd .. || exit 1 # return to root dir or exit on failure
	./build/Tictoc #run build executable
}


# if build folder exist run build process 
if  [[ -d $build_dir ]]; then
	build_process
else
	echo "creating build directory ..."
	mkdir build || exit 1 # create dir or exit on failure
	echo "directory created"
	build_process
fi


