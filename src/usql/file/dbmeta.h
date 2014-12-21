/*
 * $File: dbmeta.h
 * $Date: Tue Oct 21 09:52:39 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include <cstring>
#include "../common.h"

namespace usql {

struct DBMeta {
    static constexpr size_t MAGIC_LEN = 16;

    char magic[MAGIC_LEN];
    size_t page_size;

    //! number of pages available in file, including this meta page
    size_t nr_page;

    size_t nr_page_allocated;

    size_t page_freelist_root;

    // for root table
    size_t root_table_page;
    rowid_t root_table_rows;
    size_t root_index_page;

    bool check_magic();

    void init(size_t page_size);
};

}   // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

