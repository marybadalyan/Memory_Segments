cmake_minimum_required(VERSION 3.10)

# Set the project name
project(memory_segments)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(memory_segments main.cpp)

# Post-build analysis per platform
if(MSVC)
    add_custom_target(dumpbin_info ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Dumping PE info (MSVC)"
        COMMAND cmd /c "dumpbin /headers \"$<TARGET_FILE:memory_segments>\" > \"${CMAKE_BINARY_DIR}/analysis/memory_segments_headers.txt\""
        COMMAND cmd /c "dumpbin /symbols \"$<TARGET_FILE:memory_segments>\" > \"${CMAKE_BINARY_DIR}/analysis/memory_segments_symbols.txt\""
        COMMAND cmd /c "dumpbin /all \"$<TARGET_FILE:memory_segments>\" > \"${CMAKE_BINARY_DIR}/analysis/memory_segments_all.txt\""
        DEPENDS memory_segments
    )
else()
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/memory_segments_headers.txt
               ${CMAKE_BINARY_DIR}/memory_segments_symbols.txt
               ${CMAKE_BINARY_DIR}/memory_segments_size.txt
        COMMAND ${CMAKE_COMMAND} -E echo "Dumping ELF info (Unix)"
        COMMAND readelf -S "$<TARGET_FILE:memory_segments>" > "${CMAKE_BINARY_DIR}/analysis/memory_segments_headers.txt"
        COMMAND objdump -t "$<TARGET_FILE:memory_segments>" > "${CMAKE_BINARY_DIR}/analysis/memory_segments_symbols.txt"
        COMMAND size "$<TARGET_FILE:memory_segments>" > "${CMAKE_BINARY_DIR}/analysis/memory_segments_size.txt"
        DEPENDS memory_segments
        COMMENT "Generating Linux ELF info"
    )
endif()
