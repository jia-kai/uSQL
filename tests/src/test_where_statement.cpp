/* 
* @Author: BlahGeek
* @Date:   2014-12-07
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-09
*/

#include "./page_io_env.h"
#include "./utils.h"

#include "usql/table.h"

#include "usql/ds/btree.h"
#include "usql/datatype/int.h"
#include "usql/datatype/string.h"
#include "usql/where_statement.h"
#include "usql/sql_statement.h"

#include <map>

using namespace usql;

namespace {

class WhereStatementTest: public PageIOTestEnv {
protected:

    std::unique_ptr<DataTypeBase> datatype0, datatype1;

    PageIO::page_id_t index0_root = 0, index1_root = 0;

    std::map<std::string, std::set<rowid_t>> rows;
    std::map<ColumnAndTableName, std::unique_ptr<IndexBase>> indexes;

    void SetUp() override {
        PageIOTestEnv::SetUp();
        datatype0 = std::make_unique<IntDataType>();
        datatype1 = std::make_unique<IntDataType>();

        // index0_root = m_page_io->alloc().id();
        // index1_root = m_page_io->alloc().id();

        indexes[ColumnAndTableName("t1", "c2")] = 
                 datatype0->load_index(*m_page_io, index0_root,
                                       [this](const PageIO::Page & root) {
                                         index0_root = root.id();
                                       });
        indexes[ColumnAndTableName("t2", "c1")] = 
                 datatype1->load_index(*m_page_io, index1_root,
                                       [this](const PageIO::Page & root) {
                                         index1_root = root.id();
                                       });
        rows["t1"].insert(WhereStatement::INCLUDE_ALL);
        rows["t2"].insert(WhereStatement::INCLUDE_ALL);

        auto & index0 = indexes[ColumnAndTableName("t1", "c2")];
        for(int i = 0 ; i < 10000 ; i += 1)
            index0->insert(LiteralData(i % 100), i + 1000);
        auto & index1 = indexes[ColumnAndTableName("t2", "c1")];
        for(int i = 0 ; i < 10000 ; i += 1)
            index1->insert(LiteralData(i), i);
    }

};


TEST_F(WhereStatementTest, basic_filter_test) {
    std::string sql("SELECT * FROM t1, t2 WHERE\n"
               "t1.c2 = 42 and t2.c1 <= 10\n"
               "and (1 < t2.c1 or t1.c2 >= 999)\n"
               "and not (t2.c1 >= 9 and t2.c1 >= 8)\n "
               "and (((1>1)or(1<1)or(2!=1)) and t1.c2 < t2.c1)\n"
               "and not t1.c1 > 0\n");
    SQLStatement stmt(sql);
    EXPECT_EQ(stmt.parse(), 0);
    auto & where_stmt = stmt.where_stmt;

    where_stmt->normalize();
    auto ret = where_stmt->filter(rows, indexes);
    EXPECT_EQ(100, ret["t1"].size());
    EXPECT_EQ(9, ret["t2"].size());
}


}
