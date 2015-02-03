set(project nap)

include (GenerateExportHeader)

file(GLOB MyFiles ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

set_source_files_properties(    
    SOURCE ${MyFiles}
    PROPERTIES COMPILE_FLAGS -std=c++11)

set(${project}_files
    # Source files
    ${CMAKE_CURRENT_SOURCE_DIR}/call_ctx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/consts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/method.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/opr_hndl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/evaluate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/expression_with_location.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_abstraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/garbage_bin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_finalizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/charconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variable_holder.cpp

    # Header files
    ${CMAKE_CURRENT_SOURCE_DIR}/call_ctx.h
    ${CMAKE_CURRENT_SOURCE_DIR}/charconverter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/code_finalizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/code_output.h
    ${CMAKE_CURRENT_SOURCE_DIR}/code_stream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common_structs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/consts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/envelope.h
    ${CMAKE_CURRENT_SOURCE_DIR}/evaluate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/expression_tree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/file_abstraction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/garbage_bin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/method.h
    ${CMAKE_CURRENT_SOURCE_DIR}/number.h
    ${CMAKE_CURRENT_SOURCE_DIR}/opr_hndl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/parameter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/res_wrds.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_brkp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/type.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/variable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/variable_holder.h

    # From VM
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

