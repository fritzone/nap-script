set(project nbci)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/*.c PROPERTIES COMPILE_FLAGS -std=c90)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/strtable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/metatbl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jmptable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/push.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nbci_impl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/comparison.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mov.c
    ${CMAKE_CURRENT_SOURCE_DIR}/marks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/clrs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/nbci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/byte_order.c
    ${CMAKE_CURRENT_SOURCE_DIR}/call.c
    ${CMAKE_CURRENT_SOURCE_DIR}/peek.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/return.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/dec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/clidx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/operation.c
)

add_library(nbci ${${project}_files} )

add_executable(napi
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

target_link_libraries(napi nbci)
