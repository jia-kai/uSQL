/*
 * $File: utils.h
 * $Date: Wed Nov 05 19:56:21 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include <cstddef>

static inline size_t pow(size_t b, size_t p) {
    size_t x = 1;
    for (size_t i = 0; i < p; i ++)
        x *= b;
    return x;
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}
