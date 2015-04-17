message("CMAKE_SRC_DIR = ${CMAKE_SOURCE_DIR}")
message("CMAKE_BIN_DIR = ${CMAKE_BINARY_DIR}")

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

add_executable(panorama
	main.cpp
	kdtree.cpp
	siftpp/sift.cpp
	panorama.cpp
	pgm.cpp
)
set(LIBS lapack ${LIBS})
target_link_libraries(panorama ${LIBS})
