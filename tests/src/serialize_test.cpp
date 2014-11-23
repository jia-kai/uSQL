/*
 * $File: serialize_test.cpp
 * $Date: Sun Nov 23 20:32:11 2014 +0800
 * $Author: jiakai <jia.kai66@gmail.com>
 */

#include "./usql/serializable.h"
#include "./utils.h"

#include <gtest/gtest.h>

#include <cstring>

using namespace usql;

namespace {

class Ser final: public Serializable {
    std::string m_data;

    size_t do_get_serialize_length() const override {
        return m_data.size();

    }

    void do_serialize(void *dest) const override {
        memcpy(dest, m_data.c_str(), m_data.size());
    }

    public:
        Ser() {
            size_t len = randi(500, 1000);
            for (size_t i = 0; i < len; i ++)
                m_data.push_back(randi(255));
        }

        Ser(const void *data, size_t length):
            m_data(static_cast<const char*>(data), length)
        {
        }

        TypeID type_id() const {
            return TypeID::GTEST_SERIALZE;
        }

        static std::unique_ptr<Serializable> deserialize(
                const void *data, size_t length) {
            return std::make_unique<Ser>(data, length);
        }

        std::string& data() {
            return m_data;
        }
};

}

REGISTER_SERIALIZABLE(Ser);

TEST(SerializationTest, Serialize) {
    Ser ser;
    auto len = ser.get_serialize_length();
    char ser_data[len];
    ser.serialize(ser_data);
}

TEST(SerializationTest, Deserialize) {
    Ser ser;
    auto len = ser.get_serialize_length();
    char ser_data[len];
    ser.serialize(ser_data);
    auto de = Serializable::deserialize(ser_data, len);
    auto ptr = dynamic_cast<Ser*>(de.get());
    EXPECT_NE(nullptr, ptr);
    EXPECT_TRUE(ptr->data() == ser.data());
}

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}

