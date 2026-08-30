#ifndef __DEFINES_HPP_INCLUDED__
#define __DEFINES_HPP_INCLUDED__

#include "assert.h"

// -- TYPES --

using fp = float;

// -- MACROS --

#define ASSERT_OR_THROW(expr)               \
if(expr) ;                                  \
else {                                      \
    assert(false);                          \
    throw std::invalid_argument("error!");  \
}               

// Standart L1 cache line size
#define L1_CACHE_LINE_SIZE 64

// Note: uncomment for debug features
// #define _DEBUG

#endif