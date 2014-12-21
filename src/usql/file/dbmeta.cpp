/*
 * $File: dbmeta.cpp
 * $Date: Tue Oct 21 09:53:06 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./dbmeta.h"

using namespace usql;

static const char MAGIC[DBMeta::MAGIC_LEN] = "uSQL db format0";

void DBMeta::init(size_t page_size_) {
    memset(this, 0, sizeof(DBMeta));
    memcpy(magic, MAGIC, MAGIC_LEN);
    page_size = page_size_;
    nr_page = 1;
    nr_page_allocated = 1;
}

bool DBMeta::check_magic() {
    return !memcmp(magic, MAGIC, MAGIC_LEN);
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

