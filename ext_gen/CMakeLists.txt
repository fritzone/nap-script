set(project nap-ext-gen)

include (GenerateExportHeader)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/nap_ext_gen.cpp
)

add_executable(${project}
    ${${project}_files}
)


add_executable(testgen
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests/generated.cpp
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_link_libraries(${project} stdc++)
endif()
