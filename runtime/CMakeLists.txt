set(project naprt)

include (GenerateExportHeader)

include_directories(../vm ../compiler)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/nap_runtime.cpp
    ${NAP_ROOT}/inc/nap_runtime.h
)

add_library(nap_rt SHARED ${${project}_files} )

include_directories(${CMAKE_SOURCE_DIR}/vm 
					${CMAKE_BINARY_DIR}/vm 
                    ${CMAKE_SOURCE_DIR}/compiler 
                    ${CMAKE_CURRENT_BINARY_DIR} 
					${CMAKE_BINARY_DIR}/compiler
)

if(vm-ip-sp-as-macros)
    add_definitions( -DVM_IPSP_AS_MACRO )
endif()

target_link_libraries(nap_rt nap_cpl_s nap_bci_s)

GENERATE_EXPORT_HEADER(nap_rt
             BASE_NAME nap_rt
             EXPORT_MACRO_NAME NAP_LIB_API
             EXPORT_FILE_NAME nap_rt_exp.h
)
