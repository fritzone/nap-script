set(project nap_iconv)

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/iconv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/relocatable.c
)


add_library(nap_iconv STATIC ${${project}_files} )
set_target_properties(nap_iconv PROPERTIES COMPILE_FLAGS "-w")
