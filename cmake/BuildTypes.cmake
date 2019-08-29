if(TOUCAN_FAULTY)
    message(STATUS "Enable Faulty")
    set(TWIST_FAULTY TRUE)
endif()

set(ASAN_COMPILE_FLAGS -fsanitize=address,undefined -fno-sanitize-recover=all)

set(TSAN_COMPILE_FLAGS -fsanitize=thread -fno-sanitize-recover=all)

if(ASAN)
    add_compile_options(${ASAN_COMPILE_FLAGS})
endif()

if(TSAN)
    add_compile_options(${TSAN_COMPILE_FLAGS})
endif()

