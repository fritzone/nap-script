include_directories(../inc ../vm ../runtime)

add_executable(vmtest
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
)

target_link_libraries(vmtest nbci naprt napcpl )
