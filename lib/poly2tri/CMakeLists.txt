cmake_minimum_required(VERSION 3.3)
project(poly2tri)
enable_language(CXX)

set(SOURCE_FILES poly2tri/common/shapes.cc
                 poly2tri/sweep/advancing_front.cc
                 poly2tri/sweep/cdt.cc
                 poly2tri/sweep/sweep.cc
                 poly2tri/sweep/sweep_context.cc)

include_directories(${CMAKE_SOURCE_DIR}/poly2tri)

if (WIN32)
  add_library(poly2tri STATIC ${SOURCE_FILES}) # Not sure why Windows needs a static lib for this
else ()
  add_library(poly2tri SHARED ${SOURCE_FILES})
endif ()
