cmake_minimum_required(VERSION 2.6)

project(
	raster
)

if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug" OR DEBUG))
	add_definitions(-DNDEBUG)
endif()

include_directories(.)

add_executable(
	raster
	main.cpp
	objread.cpp
	raster.cpp
	transform.cpp
)
