cmake_minimum_required(VERSION 3.3)
project(demoloops)

set(ASSET_LOOPS
  test4
)

set(LOOPS ${ASSET_LOOPS}
  loop1
  loop2
  loop3
  loop4
)

# SDL2
if (EMSCRIPTEN)
  add_definitions("-std=c++11 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2")

  include_directories("/Users/tanner/code/c/SDL2_gfx-1.0.1")
else()
  include(FindPkgConfig)

  pkg_search_module(SDL2 REQUIRED sdl2)
  pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
  pkg_search_module(SDL2TFF REQUIRED SDL2_ttf>=2.0.0)
  pkg_search_module(SDL2GFX REQUIRED SDL2_gfx>=1.0.1)

  include_directories(${SDL2_INCLUDE_DIRS}
                      ${SDL2IMAGE_INCLUDE_DIRS}
                      ${SDL2TTF_INCLUDE_DIRS}
                      ${SDL2GFX_INCLUDE_DIRS})
endif(EMSCRIPTEN)

# DEMOLOOP LIB
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/demoloop-lib)
include_directories(${PROJECT_SOURCE_DIR}/lib/demoloop-lib/include)

include_directories(${PROJECT_SOURCE_DIR}/src)

foreach(LOOP ${LOOPS})
  add_executable(${LOOP} ${PROJECT_SOURCE_DIR}/src/${LOOP}.cpp)

  if (EMSCRIPTEN)
    if (${LOOP} IN_LIST ASSET_LOOPS)
      get_target_property(TEMP ${LOOP} LINK_FLAGS)
      if(TEMP STREQUAL "TEMP-NOTFOUND")
        SET(TEMP "") # set to empty string
      endif()
      # append our values
      SET(TEMP "${TEMP} --preload-file ../res --use-preload-plugins")
      set_target_properties(${LOOP} PROPERTIES LINK_FLAGS ${TEMP} )
    endif()

    target_link_libraries(${LOOP} demoloop-lib "/Users/tanner/code/c/SDL2_gfx-1.0.1/build/.libs/libSDL2_gfx.a")
    set_property(TARGET ${LOOP} PROPERTY SUFFIX ".html")
  else()
    target_link_libraries(${LOOP} demoloop-lib
                          ${SDL2_LIBRARIES}
                          ${SDL2IMAGE_LIBRARIES}
                          ${SDL2TFF_LIBRARIES}
                          ${SDL2GFX_LIBRARIES})
    set_property(TARGET ${LOOP} PROPERTY CXX_STANDARD 11)
    add_custom_target(run-${LOOP}
        COMMAND ${LOOP}
        DEPENDS ${LOOP}
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    )
  endif(EMSCRIPTEN)
endforeach(LOOP)
