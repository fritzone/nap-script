set(interrupt_files "" CACHE INTERNAL "interrupt_files")

if( enable-runtime-compilation )
	add_subdirectory(intr_2)
	add_subdirectory(intr_3)
endif()

add_subdirectory(intr_4)
