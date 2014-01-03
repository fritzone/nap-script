set(project naprt)

include_directories(../vm ../compiler)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/nap_runtime.cpp
)

add_library(nap-rt SHARED ${${project}_files} )
add_library(nap-rt-s STATIC ${${project}_files} )

include_directories(${CMAKE_SOURCE_DIR}/vm 
					${CMAKE_BINARY_DIR}/vm 
                    ${CMAKE_SOURCE_DIR}/compiler 
                    ${CMAKE_CURRENT_BINARY_DIR} 
					${CMAKE_BINARY_DIR}/compiler
)

target_link_libraries(nap-rt nap-cpl-s nap-bci-s)
target_link_libraries(nap-rt-s nap-cpl-s nap-bci-s)

