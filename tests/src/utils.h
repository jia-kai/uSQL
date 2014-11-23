/*
 * $File: utils.h
 * $Date: Sun Nov 23 20:18:58 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include <cstddef>
#include <cstdlib>

static inline size_t pow(size_t b, size_t p) {
    size_t x = 1;
    for (size_t i = 0; i < p; i ++)
        x *= b;
    return x;
}

static inline int randi(int v) {
    return rand() / (RAND_MAX + 1.0) * v;
}

static inline size_t randi(size_t v) {
    return rand() / (RAND_MAX + 1.0) * double(v);
}

static inline size_t randi(size_t begin, size_t end) {
    return rand() / (RAND_MAX + 1.0) * double(end - begin) + begin;
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}
