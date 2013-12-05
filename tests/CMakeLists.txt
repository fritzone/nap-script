include_directories(../inc ../vm ../runtime ../compiler ./gtest/include)

add_executable(vmtest
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest/src/gtest_main.cc
)
if(WIN32)
    add_definitions("/D_VARIADIC_MAX=10")
endif()

add_subdirectory(gtest)

target_link_libraries(vmtest nbci naprt napcpl gtest)

