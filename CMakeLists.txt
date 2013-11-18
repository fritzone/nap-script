cmake_minimum_required (VERSION 2.6)
project (nap-script)

option(coverage-build "gcov/lcov test coverage analysis (make coverage_test)" OFF)

if(NOT WIN32)
  add_definitions(-Wall -Wextra )
else()
  add_definitions(/W3)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(inc vm)

if(coverage-build)
   set(CMAKE_BUILD_TYPE Debug)
   message("building for coverage ...")
   SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
   SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
   
   include(cmake/CodeCoverage.cmake)
   SETUP_TARGET_FOR_COVERAGE(coverage_test ctest coverage)
endif()

add_subdirectory(compiler)
add_subdirectory(vm)
add_subdirectory(runtime)
add_subdirectory(tests)

enable_testing()

add_test(NAME nap_vm_test COMMAND vmtest)

