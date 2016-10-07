cmake_minimum_required(VERSION 2.8)
project(demoloops)

set(LOOPS
  test1
  test2
  test4
)

# SDL2
if (EMSCRIPTEN)
  add_definitions("-std=c++11 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ../res --use-preload-plugins")

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

# DEMOLOOP LIB
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/demoloop-lib)
include_directories(${PROJECT_SOURCE_DIR}/lib/demoloop-lib/include)

include_directories(${PROJECT_SOURCE_DIR}/src)

foreach(LOOP ${LOOPS})
  add_executable(${LOOP} ${PROJECT_SOURCE_DIR}/src/${LOOP}.cpp)
  target_link_libraries(${LOOP} demoloop-lib
                        ${SDL2_LIBRARIES}
                        ${SDL2IMAGE_LIBRARIES}
                        ${SDL2TFF_LIBRARIES})

  if (EMSCRIPTEN)
    set_property(TARGET ${LOOP} PROPERTY SUFFIX ".html")
  else()
    set_property(TARGET ${LOOP} PROPERTY CXX_STANDARD 11)
    add_custom_target(run-${LOOP}
        COMMAND ${LOOP}
        DEPENDS ${LOOP}
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    )
  endif(NOT EMSCRIPTEN)
endforeach(LOOP)
