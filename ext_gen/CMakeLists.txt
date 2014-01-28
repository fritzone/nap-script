set(project nap-ext-gen)

include (GenerateExportHeader)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/nap_ext_gen.cpp
)

add_executable(${project}
    ${${project}_files}
)

MESSAGE(${CMAKE_CURRENT_BINARY_DIR})
