cmake_minimum_required(VERSION 3.0.0)
project(subdivisions VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 11)

# Add Include Path
include_directories(
	head
	include
)

# find source codes
# Add source files
file(GLOB_RECURSE SOURCE_FILES 
    src/*.cpp
    head/*.h
    src/**/*.cpp
    head/**/*.h
)
if(WIN32)
add_definitions(-D_AMD64_)
endif()
add_executable(subdivisions ${SOURCE_FILES})
