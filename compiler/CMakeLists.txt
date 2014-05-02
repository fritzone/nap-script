set(project nap)

include (GenerateExportHeader)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/call_ctx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/consts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/method.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/opr_hndl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/type.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/evaluate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_abstraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/garbage_bin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_finalizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/charconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameter.cpp
    ${CMAKE_SOURCE_DIR}/vm/byte_order.c
)

if(enable-runtime-compilation)
    add_definitions( -DRUNTIME_COMPILATION )
endif()

include_directories(${CMAKE_SOURCE_DIR}/vm 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/compiler
)

list(APPEND compiler_files ${${project}_files})
set(compiler_files ${${project}_files} CACHE INTERNAL "compiler_files" )

add_library(nap_cpl_s STATIC ${${project}_files} )
add_executable(nap
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

LINK_DIRECTORIES(${NAP_ROOT}/vm/$(ConfigurationName)/)

target_link_libraries(nap_cpl_s nap_bci_s)

target_link_libraries(nap nap_bci_s nap_cpl_s)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_link_libraries(nap_cpl_s stdc++)
endif()

