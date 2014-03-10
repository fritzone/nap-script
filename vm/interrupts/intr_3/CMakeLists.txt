set(project nbci-intr_3)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/intr_3.c
)

list(APPEND interrupt_files ${${project}_files} )
set(interrupt_files ${interrupt_files} CACHE INTERNAL "interrupt_files" )

