/*
 * $File: int.h
 * $Date: Sun Nov 23 20:14:45 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "./base.h"

namespace usql {

class IntDataType final: public DataTypeBase {

    /*
     * the length seems to be useless
     * (https://alexander.kirk.at/2007/08/24/what-does-size-in-intsize-of-mysql-mean/)
     * so we have nothing so serialize here
     */
    size_t do_get_serialize_length() const override {
        return 0;
    }

    void do_serialize(void * /*dest*/) const override {
    }
    
    public:
        TypeID type_id() const override {
            return TypeID::DATATYPE_INT;
        }

        size_t storage_size() const override {
            return sizeof(int);
        }
};

} // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

