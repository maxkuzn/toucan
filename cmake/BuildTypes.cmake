set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fno-sanitize-recover=all"
    CACHE STRING "Flags used during ASAN builds"
    FORCE)

