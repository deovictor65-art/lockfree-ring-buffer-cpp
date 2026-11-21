cmake_minimum_required(VERSION 3.20)
project(lockfree_ring_buffer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

if (MSVC)
    add_compile_options(/W4 /O2 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
endif()

add_library(lockfree_core INTERFACE)
target_include_directories(lockfree_core INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_executable(lockfree_bench src/main.cpp)
target_link_libraries(lockfree_bench PRIVATE lockfree_core Threads::Threads)

enable_testing()
add_executable(lockfree_tests tests/test_lockfree.cpp)
target_link_libraries(lockfree_tests PRIVATE lockfree_core Threads::Threads)

add_test(NAME LockFreeTests COMMAND lockfree_tests)
