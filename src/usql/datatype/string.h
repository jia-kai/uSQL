#pragma once

#include "./base.h"

namespace usql {

class StringData;

class StringDataType: virtual public DataTypeBase {
protected:
    size_t max_size = 0;

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

    std::unique_ptr<DataBase> load(const void * src) override;

};

class StringData: public DataBase, public StringDataType {
    friend class StringDataType;
private:
    std::string val;
    StringData(std::string val): val(val) {}

public:
    StringData() = default;

    void dump(void * dest) const override {
        strncpy(static_cast<char *>(dest), val.c_str(), max_size);
    }

    static bool compare(const DataBase * a, const DataBase * b) {
        #define HASH(x) (std::hash<std::string>()(x))
        return HASH(static_cast<const StringData *>(a)->val) < HASH(static_cast<const StringData *>(b)->val);
    }
};

}

