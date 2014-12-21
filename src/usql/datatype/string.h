#pragma once

#include "./base.h"
#include <functional>

namespace usql {

class StringDataType: virtual public DataTypeBase {
protected:
    size_t max_size = 256;

    LiteralData do_load(const void * src) const override {
        LiteralData ret;
        ret.string_v = std::string(static_cast<const char *>(src), 0, max_size);
        return ret;
    }

    void do_dump(void * dest, const LiteralData & data) const override{
        strncpy(static_cast<char *>(dest), data.string_v.c_str(), max_size);
    }

public:
    StringDataType() = default;
    StringDataType(size_t size_): max_size(size_) {}

    size_t storage_size() const override {
        return max_size;
    }

    DataType type_id() const override {
        return DataType::STRING;
    }

    std::string type_name() const override {
        return ssprintf("VARCHAR(%zd)", max_size);
    }

    LiteralData make_default() const override {
        return LiteralData(std::string(""));
    }

public:

    using hash_result_type = std::hash<std::string>::result_type;

    std::shared_ptr<IndexBase> load_index(
        PageIO &page_io, PageIO::page_id_t root, 
        PagedDataStructureBase::root_updator_t root_updator) override {

        return std::make_shared<Index<hash_result_type>>(
            [](const LiteralData & data)->hash_result_type{
                return std::hash<std::string>()(data.string_v);
            },
            page_io, root, root_updator);
    }

};


}

