
include_directories(../inc)

add_executable (test test.c)

target_link_libraries(test nbci)