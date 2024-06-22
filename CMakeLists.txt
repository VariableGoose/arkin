cmake_minimum_required(VERSION 3.29)
project(arkin)

option(ARKIN_BUILD_TESTS "Build tests." OFF)
option(ARKIN_BUILD_SHARED_LIB "Build shared library." OFF)
option(ARKIN_DEBUG "Debug build mode." OFF)
option(ARKIN_SANITIZE_ADDRESSES "Enable address sanitizer." OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O1 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O3")

if (ARKIN_DEBUG)
    set(CMAKE_BUILD_TYPE debug)
else ()
    set(CMAKE_BUILD_TYPE release)
endif ()

if (ARKIN_BUILD_SHARED_LIB)
    set(ARKIN_LIB_TYPE SHARED)
else ()
    set(ARKIN_LIB_TYPE STATIC)
endif ()

add_library(arkin ${ARKIN_LIB_TYPE}
    src/arkin_core.c
    src/arkin_log.c
    src/arkin_test.c)

if (ARKIN_SANITIZE_ADDRESSES)
    target_compile_definitions(arkin PRIVATE "ARKIN_SANITIZE_ADDRESSES")
    target_compile_options(arkin PRIVATE "-fsanitize=address" "-fno-omit-frame-pointer")
    target_link_options(arkin PRIVATE "-fsanitize=address" "-fno-omit-frame-pointer")
endif ()

target_include_directories(arkin PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

if (ARKIN_BUILD_TESTS)
    set(ARKIN_SANITIZE_ADDRESSES ON)
    set(ARKIN_DEBUG ON)

    add_executable(test
        tests/main.c
        tests/core.c
        tests/linked_lists.c
        tests/strings.c
        tests/hash_map.c
        tests/pool.c
    )
    target_link_libraries(test arkin)
    target_include_directories(test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests/")

    target_compile_options(test PRIVATE "-fsanitize=address" "-fno-omit-frame-pointer")
    target_link_options(test PRIVATE "-fsanitize=address" "-fno-omit-frame-pointer")
endif ()
