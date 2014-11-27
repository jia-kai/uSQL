/* 
* @Author: BlahGeek
* @Date:   2014-11-27
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-11-27
*/

#include <iostream>
#include "./usql/serializable.h"
#include "./usql/datatype/int.h"
#include "./utils.h"

#include <gtest/gtest.h>

#include <cstring>
#include <thread>
#include <future>
#include <vector>

using namespace usql;

namespace {

class SerNoDeseialize final: public Serializable {
    size_t do_get_serialize_length() const override {
        return 0;
    }

    void do_serialize(void *dest) const override {
    }

    public:
        TypeID type_id() const {
            return TypeID::GTEST_SERIALZE_NO_DESERIALIZE;
        }
};

}

REGISTER_SERIALIZABLE(SerNoDeseialize);

TEST(SerializationTest, DeserializeNoImplement) {
    SerNoDeseialize ser;
    auto len = ser.get_serialize_length();
    char ser_data[len];
    ser.serialize(ser_data);
    ASSERT_DEATH(Serializable::deserialize(ser_data, len), "recursive *");
}
