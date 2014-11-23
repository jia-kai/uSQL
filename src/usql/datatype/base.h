/*
 * $File: base.h
 * $Date: Sun Nov 23 20:15:01 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "../serializable.h"

namespace usql {

class DataTypeBase: public Serializable {
    protected:

    public:

        /*!
         * get storage size used in each row, which must be constant regardless
         * of the content
         */
        virtual size_t storage_size() const = 0;
};


/*!
 * literal data type decoded from the query
 */
struct LiteralDataType {
    enum Type {
        INT, STRING
    };
    Type type;
    int int_v;
    std::string str_v;
};

} // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

