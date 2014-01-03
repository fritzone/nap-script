cmake_minimum_required (VERSION 2.6)
project (nap-script)

option(coverage-build "gcov/lcov test coverage analysis (make coverage_test)" OFF)

if(NOT WIN32)
  add_definitions(-Wall -Wextra)
  option(gprof-build "enable gprof" OFF)
  if(gprof-build)
    add_definitions("-pg")
  endif()
else()
  add_definitions(/W3)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
  add_definitions(/D_SCL_SECURE_NO_WARNINGS)
endif()

include_directories(inc vm)

if(coverage-build)
   set(CMAKE_BUILD_TYPE Debug)
   message("building for coverage ...")
   SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -rdynamic")
   SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -rdynamic")

   include(cmake/CodeCoverage.cmake)
   SETUP_TARGET_FOR_COVERAGE(coverage_test ctest coverage)
endif()

if(gprof-build)
  set(CMAKE_EXE_LINKER_FLAGS "-pg")
endif()

set(NAP_ROOT ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "NAP_ROOT")

set(compiler_files CACHE INTERNAL "compiler_files" FORCE)
add_subdirectory(compiler)

add_subdirectory(vm)
add_subdirectory(runtime)
add_subdirectory(tests)

enable_testing()

add_test(NAME nap_vm_test COMMAND vmtest)

