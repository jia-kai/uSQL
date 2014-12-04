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

class DataBase;

class DataTypeBase {
    protected:

    public:

        virtual ~DataTypeBase() = default;

        /*!
         * get storage size used in each row, which must be constant regardless
         * of the content
         */
        virtual size_t storage_size() const = 0;

        virtual DataType type_id() const = 0;
        virtual std::string type_name() const = 0;

        virtual std::unique_ptr<DataBase> load(const void * src) = 0;

};

class DataCmp;

class DataBase : virtual public DataTypeBase {
public:
    virtual ~DataBase() {};
    virtual void dump(void * dest) const = 0;

    friend class DataCmp;

};

class DataCmp {
public:

    using compare_function_t = std::function<
        bool(const DataBase *, const DataBase *)>;

    bool operator () (const DataBase & a, const DataBase & b) const;

    static void register_datatype(DataType type_id, compare_function_t comp_f);
};

#define REGISTER_DATATYPE(cls) \
    namespace { namespace __datatype_ctor_##cls { \
        static_assert(std::is_base_of<::usql::DataBase, cls>::value, \
                "must be derived from DataBase"); \
        class Ctor { \
            public: \
                Ctor() { \
                    cls v; \
                    ::usql::DataCmp::register_datatype(\
                            v.type_id(), cls::compare); \
                } \
        }; \
        Ctor ctor; \
    }}

} // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

