cmake_minimum_required (VERSION 2.8)

set(PROJECT_NAME libnestegg)
project(${PROJECT_NAME} C)

# sources
set(SOURCES
	src/nestegg.c
)
set(HEADERS
	include/nestegg/nestegg.h
)

# includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# make library
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

