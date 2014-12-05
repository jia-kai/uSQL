/*
 * $File: int.h
 * $Date: Sun Nov 23 20:14:45 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "./base.h"
#include <iostream>

namespace usql {

class IntDataType: virtual public DataTypeBase {

protected:
    LiteralData do_load(const void * src) const override{
        LiteralData ret;
        ret.int_v = *(const int64_t *)src;
        return ret;
    }

    void do_dump(void * dest, const LiteralData & data) const override{
        int64_t * p = static_cast<int64_t *>(dest);
        *p = data.int_v;
    }

public:

    size_t storage_size() const override {
        return sizeof(int64_t);
    }

    DataType type_id() const override {
        return DataType::INT;
    }

    std::string type_name() const override {
        return "INT";
    }

};

} // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

