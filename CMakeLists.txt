# -*- mode: cmake -*-
# vi: set ft=cmake :

cmake_minimum_required(VERSION 3.5.1)
project(drake-schunk-driver)

list(INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake/modules")

if(CYGWIN OR NOT UNIX)
  message(FATAL_ERROR "Cygwin and non-Unix platforms are NOT supported")
endif()

find_package(Bazel 0.6.1 MODULE REQUIRED)

get_filename_component(C_COMPILER_REALPATH "${CMAKE_C_COMPILER}" REALPATH)
get_filename_component(C_COMPILER_NAME "${C_COMPILER_REALPATH}" NAME)

get_filename_component(CXX_COMPILER_REALPATH "${CMAKE_CXX_COMPILER}" REALPATH)
get_filename_component(CXX_COMPILER_NAME "${CXX_COMPILER_REALPATH}" NAME)

if(C_COMPILER_NAME STREQUAL ccache OR CXX_COMPILER_NAME STREQUAL ccache)
  message(FATAL_ERROR
    "Compilation with ccache is NOT supported due to incompatibility with Bazel"
  )
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are None Debug Release RelWithDebInfo MinSizeRel"
    FORCE
  )
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
    STRINGS None Debug Release RelWithDebInfo MinSizeRel
  )
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(BAZEL_COMPILATION_MODE dbg)
else()
  set(BAZEL_COMPILATION_MODE opt)
endif()

set(BAZEL_ENV "CC=${CMAKE_C_COMPILER}" "CXX=${CMAKE_CXX_COMPILER}")

set(BAZEL_ARGS
  --compilation_mode=${BAZEL_COMPILATION_MODE}
  --cxxopt=${CMAKE_CXX14_STANDARD_COMPILE_OPTION}
  --host_cxxopt=${CMAKE_CXX14_STANDARD_COMPILE_OPTION}
)

if(CMAKE_VERBOSE_MAKEFILE)
  list(APPEND BAZEL_ARGS --subcommands)
endif()

include(ExternalProject)

set(BAZEL_TARGETS //...)

ExternalProject_Add(drake-schunk-driver
  SOURCE_DIR "${PROJECT_SOURCE_DIR}"
  CONFIGURE_COMMAND :
  BUILD_COMMAND ${BAZEL_ENV} "${Bazel_EXECUTABLE}" build ${BAZEL_ARGS} ${BAZEL_TARGETS}
  BUILD_IN_SOURCE 1
  BUILD_ALWAYS 1
  INSTALL_COMMAND :
  USES_TERMINAL_BUILD 1
  USES_TERMINAL_INSTALL 1
)
