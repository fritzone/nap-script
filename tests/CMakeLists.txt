
include_directories(
    ../inc ../vm
    ../runtime ../compiler ./gtest/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(vmtest
    ${CMAKE_CURRENT_SOURCE_DIR}/code_exec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/var_def.cpp
    ${NAP_ROOT}/build/tests/gen_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest/src/gtest_main.cc
)


add_dependencies(vmtest testgen)

if(WIN32)
    add_definitions("/D_VARIADIC_MAX=10")
endif()

if(WIN32)
  add_custom_command(
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/generated.cpp
    OUTPUT ${NAP_ROOT}/build/tests/gen_test.cpp
    COMMAND testgen
    COMMENT "Running TESTGEN"
    VERBATIM
  )
else()
  add_custom_command(
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/generated.cpp
    OUTPUT ${NAP_ROOT}/build/tests/gen_test.cpp
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../ext_gen/testgen
    COMMENT "Running TESTGEN"
    VERBATIM
  )
endif()


include_directories(${CMAKE_BINARY_DIR}/compiler 					
					${CMAKE_BINARY_DIR}/runtime
)

add_subdirectory(gtest)

target_link_libraries(vmtest nap_rt nap_bci_s nap_cpl_s gtest)

