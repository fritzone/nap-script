cmake_minimum_required (VERSION 2.6)
project (nap-script)

if(NOT WIN32)
  add_definitions(-Wall -Wextra )
else()
  add_definitions(/W3)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(inc)

add_subdirectory(compiler)
add_subdirectory(vm)
add_subdirectory(runtime)
add_subdirectory(tests)

enable_testing()
add_test(NAME nap_vm_test COMMAND vmtest)
