/*
 * $File: dbmeta.h
 * $Date: Mon Oct 20 08:33:51 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include <cstring>

struct DBMeta {
    static constexpr size_t MAGIC_LEN = 16;

    char magic[MAGIC_LEN];
    size_t page_size;

    //! number of pages available in file, including this meta page
    size_t nr_page;

    size_t nr_page_allocated;

    size_t page_freelist_root;

    bool check_magic();

    void init(size_t page_size);
};

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

