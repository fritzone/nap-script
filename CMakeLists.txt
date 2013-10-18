cmake_minimum_required (VERSION 2.6)
project (nap-script)

add_definitions(-Wall)

if(NOT WIN32)
  add_definitions(-Wextra)
endif()

include_directories(inc)

add_executable(nap 
    call_ctx.cpp
    compiler.cpp
    consts.cpp
    hash.cpp
    interpreter.cpp
    method.cpp
    opr_hndl.cpp
    parser.cpp
    throw_error.cpp
    notimpl.cpp
    tree.cpp
    type.cpp
    utils.cpp
    variable.cpp
    evaluate.cpp
    code_output.cpp
    code_stream.cpp
)

set_source_files_properties(nbci.c PROPERTIES COMPILE_FLAGS -std=c90)

add_executable(napi
    nbci/nbci.c
)

