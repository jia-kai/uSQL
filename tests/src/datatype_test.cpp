/* 
* @Author: BlahGeek
* @Date:   2014-11-30
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <string>
#include "usql/datatype/int.h"
#include "usql/datatype/string.h"

#include <gtest/gtest.h>

#include <cstring>
#include <thread>
#include <future>
#include <vector>

using namespace usql;

TEST(DataTypeTest, IntData) {
    int64_t a = 42, b = 43;
    auto inttype = new IntDataType;
    auto pa = inttype->load(&a);
    auto pb = inttype->load(&b);

    EXPECT_TRUE(pa.int_v == a);
    EXPECT_TRUE(pb.int_v == b);

    EXPECT_TRUE(pa.datatype == DataType::INT);
    EXPECT_TRUE(pb.datatype == DataType::INT);

}

TEST(DataTypeTest, StringData) {
    char a[] = "abcd", b[] = "abcd", c[] = "dddddd";
    auto stringtype = new StringDataType(255);

    auto pa = stringtype->load(a);
    auto pb = stringtype->load(b);
    auto pc = stringtype->load(c);

    EXPECT_TRUE(pa.string_v == a);
    EXPECT_TRUE(pc.datatype == DataType::STRING);
}
