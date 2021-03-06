# ---------------------------------------------------------------------------
#                         Copyright Joe Drago 2018.
#         Distributed under the Boost Software License, Version 1.0.
#            (See accompanying file LICENSE_1_0.txt or copy at
#                  http://www.boost.org/LICENSE_1_0.txt)
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.5) # This is so high because of libwebp
project(colorist)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

if(COLORIST_EMSCRIPTEN)
  set(CMAKE_BUILD_TYPE "Release")
  add_definitions(-DCOLORIST_EMSCRIPTEN=1)
  if(COLORIST_EMSCRIPTEN_ASYNC)
    add_definitions(-DCOLORIST_EMSCRIPTEN_ASYNC=1)
  endif()
else()
  if(NOT CMAKE_BUILD_TYPE)
      set(CMAKE_BUILD_TYPE "Debug")
  endif()
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DCOLORIST_DEBUG=1")

if(UNIX)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
  add_definitions(-DANSI=1 -D__ANSI__=1) # for jxrlib

  if(COLORIST_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    # set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -fprofile-instr-generate")
  endif()
endif()

if(WIN32)
  add_definitions(/wd4996)
endif()

find_package(Git)
if(GIT_FOUND)
  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(NOT GIT_COMMIT)
    set(GIT_COMMIT "Unknown")
  endif()
  add_definitions(-DGIT_COMMIT="${GIT_COMMIT}")
endif()

if(HOMEBREW_BUILD)
  add_definitions(-DHOMEBREW_BUILD=1)
endif()

add_subdirectory(ext)
include_directories(${COLORIST_EXT_INCLUDES})

add_subdirectory(lib)
include_directories(lib/include)

add_subdirectory(bin)

set(LLVM_PREFIX "")
if(APPLE)
  set(LLVM_PREFIX "/usr/bin/xcrun")
endif()

if(COLORIST_COVERAGE)
  add_custom_target(colorist-test-ccov-preprocessing
      COMMAND LLVM_PROFILE_FILE=colorist-test.profraw $<TARGET_FILE:colorist-test>
      COMMAND ${LLVM_PREFIX} llvm-profdata merge -sparse colorist-test.profraw -o colorist-test.profdata
      DEPENDS colorist-test)

  add_custom_target(colorist-test-ccov-show
      COMMAND ${LLVM_PREFIX} llvm-cov show $<TARGET_FILE:colorist-test> -instr-profile=colorist-test.profdata -show-line-counts-or-regions
      DEPENDS colorist-test-ccov-preprocessing)

  add_custom_target(colorist-test-ccov-report
      COMMAND ${LLVM_PREFIX} llvm-cov report $<TARGET_FILE:colorist-test> -instr-profile=colorist-test.profdata
      DEPENDS colorist-test-ccov-preprocessing)

  add_custom_target(colorist-test-ccov
      COMMAND ${LLVM_PREFIX} llvm-cov show $<TARGET_FILE:colorist-test> -instr-profile=colorist-test.profdata -show-line-counts-or-regions -output-dir=${CMAKE_BINARY_DIR}/colorist-test-llvm-cov -format="html"
      DEPENDS colorist-test-ccov-preprocessing)

  add_custom_command(TARGET colorist-test-ccov POST_BUILD
      COMMAND ;
      COMMENT "Open ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}-llvm-cov/index.html in your browser to view the coverage report."
  )
endif()
