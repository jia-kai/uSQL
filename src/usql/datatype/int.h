/*
 * $File: int.h
 * $Date: Sun Nov 23 20:14:45 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "./base.h"
#include <iostream>

namespace usql {

class IntData;

class IntDataType: virtual public DataTypeBase {

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

    std::unique_ptr<DataBase> load(const void * src) override;

};

class IntData: public IntDataType, public DataBase  {

    friend class IntDataType;

private:
    int64_t val = 0;
    IntData(int64_t val): val(val) {} 

public:
    IntData() = default;
    void dump(void * dest) const override {
        int64_t * p = static_cast<int64_t *>(dest);
        *p = val;
    }

    static bool compare(const DataBase * a, const DataBase * b) {
        return static_cast<const IntData *>(a)->val < static_cast<const IntData *>(b)->val;
    }

};

} // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

