cmake_minimum_required (VERSION 2.6)
project (nap-script)

add_definitions(-Wall)

if(NOT WIN32)
  add_definitions(-Wextra)
endif()

add_executable(nap 
    bt_string.cpp
    call_ctx.cpp
    compiler.cpp
    consts.cpp
    hash.cpp
    interpreter.cpp
    is.cpp
    method.cpp
    number.cpp
    opr_hndl.cpp
    parser.cpp
    preverify.cpp
    throw_error.cpp
    notimpl.cpp
    tree.cpp
    type.cpp
    utils.cpp
    variable.cpp
    evaluate.cpp
    res_wrds.cpp
    code_output.cpp
    code_stream.cpp
)

add_executable(napi
    nbci.c
)
