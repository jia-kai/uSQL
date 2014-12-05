#pragma once

#include "./base.h"

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

};


}

