cmake_minimum_required(VERSION 3.3)
project(demoloop-lib)

set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SRCS
  ${PROJECT_SOURCE_DIR}/demoloop.cpp
  ${PROJECT_SOURCE_DIR}/helpers.cpp
  ${PROJECT_SOURCE_DIR}/hsl.cpp
)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")

# SDL2
if (EMSCRIPTEN)
  add_definitions("-std=c++11 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2")

  set(SDL2_LIBRARIES "-s USE_SDL=2")
  set(SDL2IMAGE_LIBRARIES "-s USE_SDL_IMAGE=2")
  set(SDL2TFF_LIBRARIES "-s USE_SDL_TTF=2")
else()
  include(FindPkgConfig)

  pkg_search_module(SDL2 REQUIRED sdl2)
  pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
  pkg_search_module(SDL2TFF REQUIRED SDL2_ttf>=2.0.0)

  include_directories(${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
endif(EMSCRIPTEN)

add_library(${PROJECT_NAME} SHARED ${SRCS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${SDL2TFF_LIBRARIES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
