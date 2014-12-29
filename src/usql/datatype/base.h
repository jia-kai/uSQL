/*
 * $File: base.h
 * $Date: Sun Nov 23 20:15:01 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "../common.h"
#include "../ds/btree.h"
#include <set>
#include <iostream>

namespace usql {

enum class DataType {
    INT, STRING,
};

#define DEF_OP(op) \
    bool operator op (const LiteralData & another) const { \
        if(datatype != another.datatype) return false; \
        switch(datatype) { \
            case DataType::INT: return int_v op another.int_v; \
            case DataType::STRING: return string_v op another.string_v; \
            default: return false; \
        } \
        return false; \
    } 

class LiteralData {
public:
    // Do not use union because we dont want to use std::string * str
    DataType datatype;

    int64_t int_v;
    std::string string_v;

    LiteralData() = default;
    LiteralData(const LiteralData & another) = default;
    LiteralData(int64_t v): datatype(DataType::INT), int_v(v) {}
    LiteralData(std::string v): datatype(DataType::STRING), string_v(v) {}

    DEF_OP(==)
    DEF_OP(!=)
    DEF_OP(<)
    DEF_OP(>)
    DEF_OP(>=)
    DEF_OP(<=)

    std::ostream & print(std::ostream & stream) const {
        switch(datatype) {
            case DataType::INT: stream << int_v ; break; 
            case DataType::STRING: stream << "\"" << string_v << "\"" ; break;
            default: break;
        }
        return stream;
    }

};

#undef DEF_OP

using indexrow_callback_t = std::function<bool(LiteralData &, rowid_t)>;

class IndexBase;

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

    virtual LiteralData make_default() const = 0;

    LiteralData load(const void * src) const;
    void dump(void * dest, const LiteralData & data) const;

    virtual std::shared_ptr<IndexBase> load_index(
        PageIO &page_io, PageIO::page_id_t root, 
        PagedDataStructureBase::root_updator_t root_updator) = 0;

};

}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

