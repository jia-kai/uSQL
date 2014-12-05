/*
 * $File: base.h
 * $Date: Sun Nov 23 20:15:01 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "../common.h"

namespace usql {

enum class DataType {
    INT, STRING,
};

typedef struct {
    // Do not use union because we dont want to use std::string * str
    DataType datatype;

    int64_t int_v;
    std::string string_v;
} LiteralData;

class DataTypeBase {
protected:
    virtual LiteralData do_load(const void * src) const = 0;
    virtual void do_dump(void * dest, const LiteralData & data) const = 0;

public:

    virtual ~DataTypeBase() = default;

    /*!
     * get storage size used in each row, which must be constant regardless
     * of the content
     */
    virtual size_t storage_size() const = 0;

    virtual DataType type_id() const = 0;
    virtual std::string type_name() const = 0;

    LiteralData load(const void * src) const;
    void dump(void * dest, const LiteralData & data) const;

};

}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

