cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Memory_Segments)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(memory_segments main.cpp)

# Post-build analysis per platform
if(MSVC)
    add_custom_target(dumpbin_info ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Dumping PE info (MSVC)"
        COMMAND cmd /c "dumpbin /headers \"$<TARGET_FILE:memory_segments>\" > \"${CMAKE_BINARY_DIR}/memory_segments_headers.txt\""
        COMMAND cmd /c "dumpbin /symbols \"$<TARGET_FILE:memory_segments>\" > \"${CMAKE_BINARY_DIR}/memory_segments_symbols.txt\""
        COMMAND cmd /c "dumpbin /all \"$<TARGET_FILE:memory_segments>\" > \"${CMAKE_BINARY_DIR}/memory_segments_all.txt\""
        DEPENDS memory_segments
    )
elseif(UNIX)
    # Get the full path to the built binary and store in a variable
    get_target_property(MEM_SEG_PATH memory_segments LOCATION)

    add_custom_target(elf_info ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Dumping ELF info (Unix)"
        COMMAND readelf -S "${MEM_SEG_PATH}" > "${CMAKE_BINARY_DIR}/memory_segments_headers.txt"
        COMMAND objdump -t "${MEM_SEG_PATH}" > "${CMAKE_BINARY_DIR}/memory_segments_symbols.txt"
        COMMAND size "${MEM_SEG_PATH}" > "${CMAKE_BINARY_DIR}/memory_segments_size.txt"
        DEPENDS memory_segments
    )
else()
    message(FATAL_ERROR "Unsupported platform for post-build analysis.")
endif()
