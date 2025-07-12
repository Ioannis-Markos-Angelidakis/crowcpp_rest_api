cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER "clang++")

project(crowdb)

add_executable(crowdb crowdb.cpp)

set_target_properties(crowdb PROPERTIES
    CXX_STANDARD 26
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(crowdb PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -fsanitize=address
)

target_link_options(crowdb PRIVATE
    -fsanitize=address
)

# Conditionally link Windows-specific libraries
if(WIN32)
    set(EXTRA_LIBS wsock32 ws2_32)
else()
    set(EXTRA_LIBS "")
endif()

target_link_libraries(crowdb PRIVATE sqlite3 ${EXTRA_LIBS})

# Copy templates directory to build directory
file(COPY "${CMAKE_SOURCE_DIR}/templates" DESTINATION "${CMAKE_BINARY_DIR}")
