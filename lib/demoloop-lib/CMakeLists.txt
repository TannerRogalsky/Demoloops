cmake_minimum_required(VERSION 3.3)
project(demoloop-lib)

set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SRCS
  ${PROJECT_SOURCE_DIR}/demoloop.cpp
  ${PROJECT_SOURCE_DIR}/helpers.cpp
  ${PROJECT_SOURCE_DIR}/opengl_helpers.cpp
  ${PROJECT_SOURCE_DIR}/hsl.cpp
  ${PROJECT_SOURCE_DIR}/common/object.cpp
  ${PROJECT_SOURCE_DIR}/graphics/gl.cpp
  ${PROJECT_SOURCE_DIR}/graphics/shader.cpp
  ${PROJECT_SOURCE_DIR}/graphics/image.cpp
  ${PROJECT_SOURCE_DIR}/graphics/mesh.cpp
  ${PROJECT_SOURCE_DIR}/graphics/quad.cpp
  ${PROJECT_SOURCE_DIR}/graphics/texture.cpp
  ${PROJECT_SOURCE_DIR}/graphics/canvas.cpp
  ${PROJECT_SOURCE_DIR}/graphics/2d_primitives.cpp
  ${PROJECT_SOURCE_DIR}/graphics/3d_primitives.cpp
  ${PROJECT_SOURCE_DIR}/audio/source.cpp
)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")

if (NOT EMSCRIPTEN)
  # OpenGL
  find_package(OpenGL REQUIRED)
  include_directories(${OPENGL_INCLUDE_DIR})

  # GLEW
  find_package(GLEW REQUIRED)
  include_directories(${GLEW_INCLUDE_DIRS})
endif()

# SDL2
if (EMSCRIPTEN)
  add_definitions("-std=c++11 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s USE_VORBIS=1 -s USE_OGG=1")

  set(SDL2_LIBRARIES "-s USE_SDL=2")
  set(SDL2IMAGE_LIBRARIES "-s USE_SDL_IMAGE=2")
  set(SDL2TFF_LIBRARIES "-s USE_SDL_TTF=2")
  set(VORBIS_LIBRARIES "-s USE_VORBIS=1")
  set(OGG_LIBRARIES "-s USE_OGG=1")
else()
  include(FindPkgConfig)

  # SDL2
  pkg_search_module(SDL2 REQUIRED sdl2)
  pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
  pkg_search_module(SDL2TFF REQUIRED SDL2_ttf>=2.0.0)

  include_directories(${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})

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
endif(EMSCRIPTEN)

# GLM
include_directories(${PROJECT_SOURCE_DIR}/../../glm)

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

if (EMSCRIPTEN)
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
  add_library(${PROJECT_NAME} STATIC ${SRCS})
else ()
  add_library(${PROJECT_NAME} SHARED ${SRCS})
endif (EMSCRIPTEN)
target_link_libraries(${PROJECT_NAME}
  ${SDL2_LIBRARIES}
  ${SDL2IMAGE_LIBRARIES}
  ${SDL2TFF_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${OPENAL_LIBRARY}
  ${VORBIS_LIBRARIES}
  ${VORBISFILE_LIBRARIES}
  ${OGG_LIBRARIES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
