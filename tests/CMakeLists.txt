include_directories(../inc ../vm ../lib)

add_executable(vmtest
        test.cpp
)

target_link_libraries(vmtest nbci napcpl)
