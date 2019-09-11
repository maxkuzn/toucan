#add_subdirectory("${PROJECT_SOURCE_DIR}/libs/benchmark" "libs/banchmark")

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)

set(ALL_BENCHES "")
set(ALL_RUN_BENCHES "")

# Find benchmark sources

file(GLOB BENCH_SOURCES "*.cpp")

# Build & run benchmarks

foreach(BENCH_SOURCE ${BENCH_SOURCES})
    get_filename_component(BENCH_NAME ${BENCH_SOURCE} NAME_WE)
    set(BENCH_TARGET "${BENCH_NAME}")

    add_executable(${BENCH_TARGET} ${BENCH_SOURCE})
    target_link_libraries(${BENCH_TARGET} benchmark toucan)

    list(APPEND ALL_BENCHES ${BENCH_TARGET})
endforeach()

# Build and run all benchmarks

add_custom_target(all_benchmarks)
add_dependencies(all_benchmarks ${ALL_BENCHES})

