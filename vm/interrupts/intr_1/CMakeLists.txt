set(project nbci-intr_1)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/intr_1.c
)

list(APPEND interrupt_files ${${project}_files} )
set(interrupt_files ${interrupt_files} CACHE INTERNAL "interrupt_files" )

