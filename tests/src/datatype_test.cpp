/* 
* @Author: BlahGeek
* @Date:   2014-11-30
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-11-30
*/

#include "./usql/datatype/int.h"
#include "./usql/datatype/string.h"

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

    EXPECT_NE(nullptr, pa);
    EXPECT_NE(nullptr, pb);

    EXPECT_TRUE(DataCmp()(*pa, *pb));
}

TEST(DataTypeTest, StringData) {
    char a[] = "abcd", b[] = "abcd", c[] = "dddddd";
    auto stringtype = new StringDataType(255);

    auto pa = stringtype->load(a);
    auto pb = stringtype->load(b);
    auto pc = stringtype->load(c);

    EXPECT_NE(nullptr, pa);
    EXPECT_NE(nullptr, pb);
    EXPECT_NE(nullptr, pc);

    DataCmp cmp;
    EXPECT_FALSE(cmp(*pa, *pb));
    EXPECT_FALSE(cmp(*pb, *pa));

    EXPECT_TRUE(cmp(*pa, *pc) ^ cmp(*pc, *pa));
}
