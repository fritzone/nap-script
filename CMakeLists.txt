cmake_minimum_required (VERSION 2.6)
project (nap-script)

include (TestBigEndian)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

option(coverage-build "gcov/lcov test coverage analysis (make coverage_test)" OFF)
option(build-vm "Build the Nap Virtual Machine" ON)
option(build-compiler "Build the Nap Compiler" ON)
option(make-tests "Enable Unit tests" ON)
option(build-runtime "Build the runtime" ON)

# Check for endianness of the system
test_big_endian(TEST_ENDIAN)
if(NOT ${TEST_ENDIAN} EQUAL 1)
    add_definitions( -DCOMPILED_ON_LITTLE_ENDIAN )
endif()

if(NOT WIN32)
  add_definitions(-Wall -Wextra -Wcomment -Wformat -Winit-self -Wno-long-long -Wmissing-include-dirs)
  option(gprof-build "enable gprof" OFF)
  if(gprof-build)
    add_definitions("-pg")
  endif()
  include_directories(inc vm)
else()
  add_definitions(/W4)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
  add_definitions(/D_SCL_SECURE_NO_WARNINGS)
  include_directories(inc vm iconv)
endif()


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

add_subdirectory(ext_gen)

if(build-compiler)
	set(compiler_files CACHE INTERNAL "compiler_files" FORCE)
	add_subdirectory(compiler)
endif()

if(WIN32)
    add_subdirectory(iconv)
endif()

if(build-vm)
	add_subdirectory(vm)
endif()

if(build-runtime)
	add_subdirectory(runtime)
endif()

if(make-tests)

	if(NOT build-compiler OR NOT build-runtime OR NOT build-vm)
		MESSAGE(FATAL_ERROR "In order to make the tests you need the compiler, the VM and the runtime")
	endif()

	add_subdirectory(tests)
	enable_testing()
	add_test(NAME nap_vm_test COMMAND vmtest)
endif()
