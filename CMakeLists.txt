# ============================================================================
# Cache — cache replacement algorithm comparison project
# ============================================================================
# The algorithms (LRU, LRU-K, LFU-K, HashLRU, HashLFU, ARC) are header-only
# C++ templates living in src/ and src/headers/. Every test / benchmark is a
# single translation unit that #includes them, so there is no compiled library
# to build — just shared include paths and threading.
cmake_minimum_required(VERSION 3.10)
project(Cache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The assert()-based tests need assertions enabled, so the default (unoptimized)
# build keeps them. For an optimized benchmark run, configure with:
#     cmake -B build -DCMAKE_BUILD_TYPE=Release
find_package(Threads REQUIRED)

# Shared configuration consumed by every target: include roots + threads.
add_library(cache_common INTERFACE)
target_include_directories(cache_common INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/headers")
target_link_libraries(cache_common INTERFACE Threads::Threads)

# ---- Tests -----------------------------------------------------------------
enable_testing()

# add_cache_test(<name> <source>): build an executable and register it with CTest.
function(add_cache_test name source)
    add_executable(${name} "${source}")
    target_link_libraries(${name} PRIVATE cache_common)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_cache_test(test_lru                 test/test_lru.cpp)
add_cache_test(test_lruk_run            test/test_lruk_run.cpp)
add_cache_test(test_individual          test/test_individual.cpp)
add_cache_test(test_hotspot             test/test_hotspot.cpp)
add_cache_test(test_arc_cache           src/ARC/test_arc_cache.cpp)
add_cache_test(test_arc_lfu             src/ARC/test_arc_lfu.cpp)
add_cache_test(test_arc_simple          test/test_arc_simple.cpp)
add_cache_test(test_arc_debug           test/test_arc_debug.cpp)
add_cache_test(test_arc_hotspot         test/test_arc_hotspot.cpp)
add_cache_test(test_arc_large_ws        test/test_arc_large_ws.cpp)
add_cache_test(test_arc_minimal_hotspot test/test_arc_minimal_hotspot.cpp)
add_cache_test(test_arc_crash_debug     test/test_arc_crash_debug.cpp)

# ---- Benchmark -------------------------------------------------------------
# The 6-algorithm hit-rate/latency benchmark sandboxes each run in a child
# process via POSIX fork()/pipe(), so it is built only on UNIX-like systems.
if(UNIX)
    add_executable(benchmark_hitrate src/benchmark_hitrate.cpp)
    target_link_libraries(benchmark_hitrate PRIVATE cache_common)
endif()
