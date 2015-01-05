set(project nbci)
include (GenerateExportHeader)
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    PROPERTIES COMPILE_FLAGS "-std=c90")

set(include_interrupt_dirs "interrupts/intr_1")

if(build-compiler)
    option(enable-runtime-compilation "Build in the runtime code compilation feature (link in intr_2 & intr_3) and enable the nap_execute() function." ON)
    if(enable-runtime-compilation)
        list(APPEND include_interrupt_dirs  "interrupts/intr_2" "interrupts/intr_3" )

        # this will go to the compiler
        add_definitions( -DRUNTIME_COMPILATION )
    endif()
else()
    unset(enable-runtime-compilation)
endif()

option(vm-ip-sp-as-macros "Generate code for defining the VM IP and SP operations to be macros instead of functions. You usually want this, unless yo debug the code." ON)
if(vm-ip-sp-as-macros)
    add_definitions( -DVM_IPSP_AS_MACRO )
endif()

option(vm-prefer-dynamic-allocation "Generate code which allocates memory on the heap instead of the stack for VM internals. This will lead to a smaller stack usage for the application but a somewhat slower startup." ON)
if(vm-prefer-dynamic-allocation)
    add_definitions( -DPREFER_DYNAMIC_ALLOCATION )
endif()

list(APPEND include_interrupt_dirs "interrupts/intr_4")

include_directories( ${include_interrupt_dirs} ${NAP_ROOT}/compiler )

if(enable-runtime-compilation AND NOT build-compiler)
    MESSAGE(FATAL_ERROR "You cannot have runtime compilation without building the compiler")
endif()

set(${project}_files
    ${CMAKE_CURRENT_SOURCE_DIR}/strtable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/metatbl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jmptable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/funtable.c
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
    ${CMAKE_CURRENT_SOURCE_DIR}/store.c
    ${CMAKE_CURRENT_SOURCE_DIR}/restore.c
    ${CMAKE_CURRENT_SOURCE_DIR}/clidx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/operation.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unary.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serve.c
    ${CMAKE_CURRENT_SOURCE_DIR}/poke.c

    ${NAP_ROOT}/compiler/charconverter.cpp

    ${NAP_ROOT}/inc/opcodes.h

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
  MAIN_DEPENDENCY ../ext_gen/nap_ext_gen.cpp
  OUTPUT ${NAP_ROOT}/build/ext_gen/nap_ext_def.c ${NAP_ROOT}/build/ext_gen/nap_ext_def.h
  COMMAND nap-ext-gen -n ${parameter_count} -d ${NAP_ROOT}/build/ext_gen
  COMMENT "Running generator: ${parameter_count} parameters"
  VERBATIM
)
add_library(nap_bci_s STATIC ${${project}_files} )

add_executable(napi
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

add_dependencies(nap_bci_s nap-ext-gen)

if(WIN32)
    set(vm_link_libs "nap_iconv_s" "kernel32")
else()
    set(vm_link_libs "dl")
endif()

if(build-compiler)
    list(APPEND vm_link_libs "nap_cpl_s")
endif()

target_link_libraries(nap_bci_s ${vm_link_libs})
target_link_libraries(napi nap_bci_s ${vm_link_libs})
