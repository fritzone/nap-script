set(project naprt)

include_directories(../vm ../compiler)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/nap_runtime.cpp
)

add_library(nap-rt SHARED ${${project}_files} )


target_link_libraries(nap-rt nap-cpl nap-bci)

