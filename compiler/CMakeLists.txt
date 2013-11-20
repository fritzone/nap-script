set(project nap)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/call_ctx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/consts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/method.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/opr_hndl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/notimpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/type.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/evaluate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/garbage_bin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_stream.cpp
    ../vm/byte_order.c
)

add_library(napcpl ${${project}_files} )

add_executable(nap
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(nap napcpl)
