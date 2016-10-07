cmake_minimum_required(VERSION 2.8)
project(demoloops)

set(LOOPS
  test1
  test2
  test4
)

# SDL2
include(FindPkgConfig)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
pkg_search_module(SDL2TFF REQUIRED SDL2_ttf>=2.0.0)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})

# DEMOLOOP LIB
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/demoloop-lib)
include_directories(${PROJECT_SOURCE_DIR}/lib/demoloop-lib/include)

include_directories(${PROJECT_SOURCE_DIR}/src)

foreach(LOOP ${LOOPS})
  add_executable(${LOOP} ${PROJECT_SOURCE_DIR}/src/${LOOP}.cpp)
  target_link_libraries(${LOOP} demoloop-lib
    ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${SDL2TFF_LIBRARIES})
  add_custom_target(run-${LOOP}
      COMMAND ${LOOP}
      DEPENDS ${LOOP}
      WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
  )
  set_property(TARGET ${LOOP} PROPERTY CXX_STANDARD 11)
endforeach(LOOP)
