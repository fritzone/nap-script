set(project nap-ext-gen)

include (GenerateExportHeader)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/nap_ext_gen.cpp
)

add_executable(${project}
    ${${project}_files}
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_link_libraries(${project} stdc++)
endif()
