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
    ${CMAKE_CURRENT_SOURCE_DIR}/garbage_bin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameter.cpp
    ${CMAKE_SOURCE_DIR}/vm/byte_order.c
)

include_directories(${CMAKE_SOURCE_DIR}/vm 
                    ${CMAKE_CURRENT_BINARY_DIR} 
					${CMAKE_BINARY_DIR}/compiler
)

list(APPEND compiler_files ${${project}_files})
set(compiler_files ${${project}_files} CACHE INTERNAL "compiler_files" )

add_library(nap-cpl SHARED ${${project}_files} )
add_library(nap-cpl-s STATIC ${${project}_files} )

add_executable(nap
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

LINK_DIRECTORIES(${NAP_ROOT}/vm/$(ConfigurationName)/)

target_link_libraries(nap-cpl nap-bci-s)
target_link_libraries(nap-cpl-s nap-bci-s)

GENERATE_EXPORT_HEADER(nap-cpl-s
             BASE_NAME nap-cpl-s
             EXPORT_MACRO_NAME NAP_LIB_API
             EXPORT_FILE_NAME nap_cpl_s_exp.h
             STATIC_DEFINE NAP_CPL_BUILT_AS_STATIC
)

GENERATE_EXPORT_HEADER(nap-cpl
             BASE_NAME nap-cpl
             EXPORT_MACRO_NAME NAP_LIB_API
             EXPORT_FILE_NAME nap_cpl_exp.h
             STATIC_DEFINE NAP_CPL_BUILT_AS_SHARED
)

target_link_libraries(nap nap-bci-s nap-cpl-s)
