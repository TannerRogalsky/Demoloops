cmake_minimum_required(VERSION 3.3)
project(demoloops)

set(ASSET_LOOPS
  asset_test
)

set(LOOPS ${ASSET_LOOPS}
  loop001
  loop002
  loop003
  loop004
  loop005
  loop006
  loop007
  loop008
  loop009
  loop010
  loop011
  loop012
  loop013
  loop014
  loop015
  loop016
  loop017
  loop018
  loop019
  loop020
  loop021
  loop022
  loop023
  loop024
  loop025
  testopengl
)

# Bump up warning levels appropriately for clang, gcc & msvc
# Also set debug/optimization flags depending on the build type. IDE users choose this when
# selecting the build mode in their IDE
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++11")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
endif()

# SDL2
if (EMSCRIPTEN)
  add_definitions("-std=c++11 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2")
else()
  include(FindPkgConfig)

  pkg_search_module(SDL2 REQUIRED sdl2)
  pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
  pkg_search_module(SDL2TFF REQUIRED SDL2_ttf>=2.0.0)

  include_directories(${SDL2_INCLUDE_DIRS}
                      ${SDL2IMAGE_INCLUDE_DIRS}
                      ${SDL2TTF_INCLUDE_DIRS})
endif(EMSCRIPTEN)

if (NOT EMSCRIPTEN)
  # OpenGL
  find_package(OpenGL REQUIRED)
  include_directories(${OPENGL_INCLUDE_DIR})

  # GLEW
  find_package(GLEW REQUIRED)
  include_directories(${GLEW_INCLUDE_DIRS})
endif()

# DEMOLOOP LIB
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/demoloop-lib)
include_directories(${PROJECT_SOURCE_DIR}/lib/demoloop-lib/include)

include_directories(${PROJECT_SOURCE_DIR}/src)

foreach(LOOP ${LOOPS})
  add_executable(${LOOP} ${PROJECT_SOURCE_DIR}/src/${LOOP}.cpp)

  if (EMSCRIPTEN)
    # if it's an asset loop, preload the assets
    if (${LOOP} IN_LIST ASSET_LOOPS)
      get_target_property(TEMP ${LOOP} LINK_FLAGS)
      if(TEMP STREQUAL "TEMP-NOTFOUND")
        SET(TEMP "") # set to empty string
      endif()
      # append our values
      SET(TEMP "${TEMP} --preload-file ../res --use-preload-plugins")
      set_target_properties(${LOOP} PROPERTIES LINK_FLAGS ${TEMP} )
    endif()

    target_link_libraries(${LOOP} demoloop-lib)
    # set_property(TARGET ${LOOP} PROPERTY SUFFIX ".html")
  else()
    target_link_libraries(${LOOP} demoloop-lib
                          ${SDL2_LIBRARIES}
                          ${SDL2IMAGE_LIBRARIES}
                          ${SDL2TFF_LIBRARIES}
                          ${OPENGL_LIBRARIES}
                          ${GLEW_LIBRARIES})
    set_property(TARGET ${LOOP} PROPERTY CXX_STANDARD 11)
    add_custom_target(run-${LOOP}
        COMMAND ${LOOP}
        DEPENDS ${LOOP}
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    )
  endif(EMSCRIPTEN)
endforeach(LOOP)
