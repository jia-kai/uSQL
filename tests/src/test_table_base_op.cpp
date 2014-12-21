/* 
* @Author: BlahGeek
* @Date:   2014-12-04
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include "./page_io_env.h"
#include "./utils.h"

#include "usql/db/table.h"

#include "usql/ds/btree.h"
#include "usql/datatype/int.h"
#include "usql/datatype/string.h"
#include "usql/exception.h"

using namespace usql;

namespace {

class TableBaseOPTest: public PageIOTestEnv {
protected:
    std::unique_ptr<Table> table = nullptr;

    PageIO::page_id_t tree_root = 0;
    rowid_t maxrow = 0;

    void SetUp() override {
        PageIOTestEnv::SetUp();

        std::vector<column_def_t> cols;
        cols.emplace_back(std::string("col0"), std::make_shared<IntDataType>());
        cols.emplace_back(std::string("col1"), std::make_shared<IntDataType>());
        cols.emplace_back(std::string("col2"), std::make_shared<StringDataType>(128));

        table = std::make_unique<Table>(*m_page_io, cols, 
            maxrow, [this](rowid_t x){maxrow = x; });
        table->load(tree_root, [this](const PageIO::Page & root){tree_root = root.id(); });
    }

};

}

TEST_F(TableBaseOPTest, basic) {
    std::vector<LiteralData> row0 {
        LiteralData(42), LiteralData(43), LiteralData("Aha!")
    };
    for(int i = 0 ; i < 10 ; i += 1) {
        auto rowid = table->insert(row0);
        EXPECT_EQ(rowid, i);
        EXPECT_EQ(maxrow, i+1);
    }
    for(int i = 0 ; i < 5 ; i += 1) {
        auto erased = table->erase(i * 2);
        EXPECT_TRUE(erased);
    }
    EXPECT_EQ(maxrow, 10);
    for(int i = 0 ; i < 5 ; i += 1) {
        auto erased = table->erase(i * 2);
        EXPECT_FALSE(erased);
    }

    auto values = table->find(0);
    EXPECT_TRUE(values.size() == 0);

    values = table->find(1);
    EXPECT_TRUE(values[1].int_v == 43);

    row0[0].int_v = 0;
    auto rowid = table->insert(row0, 1);
    EXPECT_EQ(rowid, 1);
    values = table->find(1);
    EXPECT_TRUE(values[0].int_v == 0);

    std::vector<LiteralData> row1 {
        LiteralData(42), LiteralData(43), LiteralData(44)
    };
    EXPECT_THROW(table->insert(row1), USQLError);

    int count = 0;
    table->walkthrough([&](rowid_t,
                const std::vector<LiteralData> &)->bool {
        count += 1;
        return false;
    });
    ASSERT_EQ(count, 5);
} 

