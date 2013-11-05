include_directories(../inc ..)

add_executable (vmtest
        test.c
        ../compiler/code_stream.cpp
        ../compiler/utils.cpp
        ../compiler/throw_error.cpp
        ../compiler/consts.cpp
)

target_link_libraries(vmtest nbci check)
