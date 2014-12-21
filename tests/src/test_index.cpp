/* 
* @Author: BlahGeek
* @Date:   2014-12-05
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include "./page_io_env.h"
#include "./utils.h"

#include "usql/db/table.h"

#include "usql/ds/btree.h"
#include "usql/datatype/int.h"
#include "usql/datatype/string.h"
#include <map>

using namespace usql;

namespace {

class IntIndexTest: public PageIOTestEnv {
protected:

    std::unique_ptr<DataTypeBase> datatype = nullptr;
    PageIO::page_id_t index_root = 0;

    std::shared_ptr<IndexBase> index = nullptr;

    void SetUp() override {
        PageIOTestEnv::SetUp();

        datatype = std::make_unique<IntDataType>();
        index = datatype->load_index(*m_page_io, index_root,
            [this](const PageIO::Page & root){index_root = root.id(); });
    }
};
    
}

TEST_F(IntIndexTest, basic) {
    for(int i = 0 ; i < 10000 ; i += 1) {
        index->insert(LiteralData(i), i);
    }
    auto result = index->find(
        IndexBase::BoundType::DISABLE,
        LiteralData(),
        IndexBase::BoundType::DISABLE,
        LiteralData());
    EXPECT_EQ(result.size(), 10000);

    result = index->find(
        IndexBase::BoundType::INCLUDE,
        LiteralData(42),
        IndexBase::BoundType::INCLUDE,
        LiteralData(42));
    EXPECT_EQ(result.size(), 1);

    result = index->find(
        IndexBase::BoundType::EXCLUDE,
        LiteralData(0),
        IndexBase::BoundType::DISABLE,
        LiteralData());
    EXPECT_EQ(result.size(), 9999);

    result = index->find(
        IndexBase::BoundType::INCLUDE,
        LiteralData(100),
        IndexBase::BoundType::INCLUDE,
        LiteralData(0));
    EXPECT_EQ(result.size(), 0);

    result = index->find(
        IndexBase::BoundType::EXCLUDE,
        LiteralData(1000),
        IndexBase::BoundType::INCLUDE,
        LiteralData(1000));
    EXPECT_EQ(result.size(), 0);

    for(int i = 0 ; i < 10000 ; i += 1) {
        index->insert(LiteralData(i), i + 10000);
    }
    result = index->find(
        IndexBase::BoundType::DISABLE,
        LiteralData(),
        IndexBase::BoundType::DISABLE,
        LiteralData());
    EXPECT_EQ(result.size(), 20000);

    result = index->find(
        IndexBase::BoundType::EXCLUDE,
        LiteralData(42),
        IndexBase::BoundType::INCLUDE,
        LiteralData(43));
    EXPECT_EQ(result.size(), 2);

    EXPECT_TRUE(result.find(43) != result.end());
    EXPECT_TRUE(result.find(10043) != result.end());

    result = index->find(
        IndexBase::BoundType::INCLUDE,
        LiteralData(1000),
        IndexBase::BoundType::EXCLUDE,
        LiteralData(1000));
    EXPECT_EQ(result.size(), 0);

}
