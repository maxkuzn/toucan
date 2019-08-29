#pragma once

#include <iostream>
#include <sstream>

static inline void MyAssert(const char* expr, const char* file, int line, const char* message) {
    std::stringstream ss;
    ss << "Assertion \"" << expr << "\" failed in file: " << file
       << ":" << line << '\n'
       << "Message: " << message << '\n';
    std::cerr << ss.str();
}

#ifdef NDEBUG
# define ASSERT(Expression, Message) \
    do {                            \
        (void)0;                    \
    } while (false)                 
#else
# define ASSERT(Expression, Message) \
    do {                            \
        if (!(Expression)) {          \
            MyAssert(#Expression,   \
                      __FILE__,     \
                      __LINE__,     \
                      Message);     \
        }                           \
    } while (false)
#endif

