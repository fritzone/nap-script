set(project nbci-intr_4)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/intr_4.c
    ${NAP_ROOT}/build/ext_gen/nap_ext_def.c
)

list(APPEND interrupt_files ${${project}_files} )
set(interrupt_files ${interrupt_files} CACHE INTERNAL "interrupt_files" )
