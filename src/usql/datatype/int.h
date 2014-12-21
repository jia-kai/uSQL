/*
 * $File: int.h
 * $Date: Sun Nov 23 20:14:45 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#pragma once

#include "./base.h"
#include "../ds/btree.h"
#include <iostream>

#include "../db/index.h"

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

    LiteralData make_default() const override {
        return LiteralData(0);
    }

private:
    using IndexTree = BTree<KeyWithRowID<int64_t>>;
    std::unique_ptr<IndexTree> index_tree = nullptr;

public:

    virtual std::shared_ptr<IndexBase> load_index(
        PageIO &page_io, PageIO::page_id_t root, 
        PagedDataStructureBase::root_updator_t root_updator) override {

        return std::make_shared<Index<int64_t>>(
            [](const LiteralData & data)->int64_t{return data.int_v;},
            page_io, root, root_updator);
    }
};

} // namespace usql

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

