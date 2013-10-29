cmake_minimum_required (VERSION 2.6)
project (nap-script)


if(NOT WIN32)
  add_definitions(-Wall -Wextra )
else()
	add_definitions(/W4)
	add_definitions(/D_CRT_SECURE_NO_WARNINGS)
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

set_source_files_properties(nbci/*.c PROPERTIES COMPILE_FLAGS -std=c90)

add_executable(napi
    nbci/nbci.c
    nbci/strtable.c
    nbci/metatbl.c
    nbci/jmptable.c
    nbci/jump.c
    nbci/push.c
    nbci/nbci_impl.c
    nbci/comparison.c
    nbci/mov.c
    nbci/marks.c
    nbci/clrs.c
    nbci/call.c
    nbci/peek.c
    nbci/pop.c
    nbci/return.c
    nbci/inc.c
    nbci/dec.c
    nbci/clidx.c
    nbci/operation.c
)

