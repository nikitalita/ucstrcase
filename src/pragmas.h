#pragma once
#ifndef NDEBUG
#include <assert.h>
#else
#define assert(x)
#endif
#if defined(__GNUC__)
#define likely(expr)    (__builtin_expect(!!(expr), 1))
#define unlikely(expr)  (__builtin_expect(!!(expr), 0))
#define noinline        __attribute__((noinline))
#elif defined(_MSC_VER)
#define likely(expr)    (expr)
#define unlikely(expr)  (expr)
#define noinline        __declspec(noinline)
#else
#define likely(expr)    (expr)
#define unlikely(expr)  (expr)
#define noinline
#endif