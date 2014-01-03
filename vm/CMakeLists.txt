set(project nbci)
include (GenerateExportHeader)
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    PROPERTIES COMPILE_FLAGS "-std=c90")

include_directories(
    interrupts/intr_2
    interrupts/intr_3
)

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

include_directories(${CMAKE_SOURCE_DIR}/vm 
                    ${CMAKE_SOURCE_DIR}/compiler 
					${CMAKE_BINARY_DIR}/vm 
                    ${CMAKE_CURRENT_BINARY_DIR} 
					${CMAKE_BINARY_DIR}/compiler
)


set(interrupt_files CACHE INTERNAL "interrupt_files" FORCE)
add_subdirectory(interrupts)
list(APPEND ${project}_files ${interrupt_files})

#add_library(nap_bci SHARED ${${project}_files} )
add_library(nap_bci_s STATIC ${${project}_files} )

#GENERATE_EXPORT_HEADER(nap_bci
#             BASE_NAME nap_bci
#             EXPORT_MACRO_NAME NAP_LIB_API
#             EXPORT_FILE_NAME nap_bci_exp.h
#)

add_executable(napi
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

target_link_libraries(nap_bci_s nap_cpl_s)
#target_link_libraries(nap_bci nap_cpl_s)

target_link_libraries(napi nap_bci_s nap_cpl_s)
