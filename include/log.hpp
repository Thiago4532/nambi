#ifndef _LOG_HPP
#define _LOG_HPP

#ifndef NDEBUG

#include <iostream>

template <typename T>
static inline
void __log(T&& fmt) {
    std::cerr << fmt << std::endl;
}

template <typename T, typename... Args>
static inline
void __log(T&& fmt, Args... args) {
    std::cerr << fmt << " ";
    __log(args...);
}

template <typename T, typename... Args>
static inline
void __log2(T&& fmt, Args... args) {
    std::cerr << "[" << fmt << "]: ";
    __log(args...);
}

#define LOG(...) __log2(__func__, __VA_ARGS__)
#else
#define LOG(...) do {} while(false)
#endif

#endif
