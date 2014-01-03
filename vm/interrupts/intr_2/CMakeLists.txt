set(project nbci-intr_2)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/intr_2.cpp
)

list(APPEND interrupt_files ${${project}_files} )
# list(APPEND interrupt_files ${compiler_files} )

set(interrupt_files ${interrupt_files} CACHE INTERNAL "interrupt_files" )
