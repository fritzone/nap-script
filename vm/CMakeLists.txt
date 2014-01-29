set(project nbci)
include (GenerateExportHeader)
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    PROPERTIES COMPILE_FLAGS "-std=c90")

include_directories(
    interrupts/intr_2
    interrupts/intr_3
    interrupts/intr_4
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
    ${CMAKE_CURRENT_SOURCE_DIR}/call_intern.c
    ${CMAKE_CURRENT_SOURCE_DIR}/peek.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/return.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/leave.c
    ${CMAKE_CURRENT_SOURCE_DIR}/dec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/clidx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/operation.c
)

include_directories(${CMAKE_SOURCE_DIR}/vm 
                    ${CMAKE_SOURCE_DIR}/compiler 
                    ${CMAKE_BINARY_DIR}/vm
                    ${CMAKE_CURRENT_BINARY_DIR} 
                    ${CMAKE_BINARY_DIR}/compiler
                    ${NAP_ROOT}/build/ext_gen/
)


set(interrupt_files CACHE INTERNAL "interrupt_files" FORCE)
add_subdirectory(interrupts)
list(APPEND ${project}_files ${interrupt_files})

set(parameter_count 7 CACHE STRING "The number of parameters the ext_gen API supports")
set(parameter_counts 2 3 4 5 6 7 8)
set_property(CACHE parameter_count PROPERTY STRINGS ${parameter_counts})

add_custom_command(
  OUTPUT ${NAP_ROOT}/build/ext_gen/nap_ext_def.c ${NAP_ROOT}/build/ext_gen/nap_ext_def.h
  COMMAND nap-ext-gen -n ${parameter_count} -d ${NAP_ROOT}/build/ext_gen
  COMMENT "Running generator"
  VERBATIM
)

add_library(nap_bci_s STATIC ${${project}_files} )

add_executable(napi
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

if(NOT WIN32)
    target_link_libraries(nap_bci_s nap_cpl_s dl)
    target_link_libraries(napi nap_bci_s nap_cpl_s dl)
else()
    target_link_libraries(nap_bci_s nap_cpl_s nap_iconv_s)
    target_link_libraries(napi nap_bci_s nap_cpl_s nap_iconv_s)
endif()
