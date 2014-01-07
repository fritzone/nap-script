set(project nap_iconv)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/iconv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/relocatable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/localcharset.c
)

add_library(nap_iconv_s STATIC ${${project}_files} )
set_target_properties(nap_iconv_s PROPERTIES COMPILE_FLAGS "-w") 
