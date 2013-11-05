set(project nap)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/call_ctx.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/consts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/method.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/opr_hndl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/throw_error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/notimpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/type.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/variable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/evaluate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code_stream.cpp
)

add_executable(nap ${${project}_files} )
