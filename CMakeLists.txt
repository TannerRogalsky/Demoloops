cmake_minimum_required(VERSION 3.8)
project(demoloops)

set(ASSET_LOOPS
  asset_test
  profile_pic
  sailor_moon
  test_openal
  hope
  sendy
  loop064
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
  loop026
  loop027
  loop028
  loop029
  loop030
  loop031
  loop032
  loop033
  loop034
  loop035
  loop036
  loop037
  loop038
  loop039
  loop040
  loop041
  loop042
  loop043
  loop044
  loop045
  loop046
  loop047
  loop048
  loop049
  loop050
  loop051
  loop052
  loop053
  loop054
  loop055
  loop056
  loop057
  loop058
  loop059
  loop060
  loop061
  loop062
  loop065
  loop066
  loop069
  mandlebrot
  uv_sphere
  normal_lighting
  normal_lighting2
  normal_lighting3
  wireframe
  loop030_3d
  testopengl
  voronoi
  lsystem
  carpenter_brut
  canvas
  two_squares_in_a_circle
  bezier
  pingpong
  parametric_surface
  snowflake
  wave_visualization
  tube
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

if(EMSCRIPTEN)
  add_definitions("-s NO_EXIT_RUNTIME=1")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# SDL2
if (EMSCRIPTEN)
  add_definitions("-std=c++1z -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s USE_VORBIS=1 -s USE_OGG=1 -s USE_BULLET=1")
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

  # OpenAL
  find_package(OpenAL REQUIRED)
  include_directories(SYSTEM ${OPENAL_INCLUDE_DIR})

  # OGG
  pkg_search_module(OGG REQUIRED ogg)
  include_directories(${OGG_INCLUDE_DIRS})

  # Vorbis
  pkg_search_module(VORBIS REQUIRED vorbis)
  include_directories(${VORBIS_INCLUDE_DIRS})

  # VorbisFile
  pkg_search_module(VORBISFILE REQUIRED vorbisfile)
  include_directories(${VORBISFILE_INCLUDE_DIRS})

  # GLEW
  find_package(GLEW REQUIRED)
  include_directories(${GLEW_INCLUDE_DIRS})

  find_package(BULLET REQUIRED)
  include_directories(SYSTEM ${BULLET_INCLUDE_DIRS})
endif()

# DEMOLOOP LIB
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/demoloop-lib)
include_directories(${PROJECT_SOURCE_DIR}/lib/demoloop-lib/include)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/poly2tri)
include_directories(${PROJECT_SOURCE_DIR}/lib/poly2tri/poly2tri)

# GLM
include_directories(${PROJECT_SOURCE_DIR}/lib/glm)

include_directories(${PROJECT_SOURCE_DIR}/src)

foreach(LOOP ${LOOPS})
  add_executable(${LOOP} ${PROJECT_SOURCE_DIR}/src/${LOOP}.cpp)

  if (EMSCRIPTEN)
    add_executable(${LOOP}_asm ${PROJECT_SOURCE_DIR}/src/${LOOP}.cpp)

    # if it's an asset loop, preload the assets
    if (${LOOP} IN_LIST ASSET_LOOPS)
      get_target_property(TEMP ${LOOP} LINK_FLAGS)
      if(TEMP STREQUAL "TEMP-NOTFOUND")
        SET(TEMP "") # set to empty string
      endif()
      # append our values
      SET(TEMP "${TEMP} --preload-file ../res/${LOOP} --use-preload-plugins")
      set_target_properties(${LOOP} PROPERTIES LINK_FLAGS ${TEMP} )
      set_target_properties(${LOOP}_asm PROPERTIES LINK_FLAGS ${TEMP} )
    endif()

    target_link_libraries(${LOOP} demoloop-lib poly2tri "-s USE_VORBIS=1 -s USE_OGG=1 -s USE_BULLET=1 -s WASM=1")
    target_link_libraries(${LOOP}_asm demoloop-lib poly2tri "-s USE_VORBIS=1 -s USE_OGG=1 -s USE_BULLET=1")

    # set_property(TARGET ${LOOP}_asm PROPERTY SUFFIX ".html")
  else()
    target_link_libraries(${LOOP} demoloop-lib poly2tri
                          ${SDL2_LIBRARIES}
                          ${SDL2IMAGE_LIBRARIES}
                          ${SDL2TFF_LIBRARIES}
                          ${OPENGL_LIBRARIES}
                          ${OPENAL_LIBRARY}
                          ${VORBIS_LIBRARIES}
                          ${VORBISFILE_LIBRARIES}
                          ${OGG_LIBRARIES}
                          ${GLEW_LIBRARIES}
                          ${BULLET_LIBRARIES})
    set_property(TARGET ${LOOP} PROPERTY CXX_STANDARD 17)
    add_custom_target(run-${LOOP}
        COMMAND ${LOOP}
        DEPENDS ${LOOP}
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    )
  endif(EMSCRIPTEN)
endforeach(LOOP)
