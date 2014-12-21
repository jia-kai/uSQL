/* 
* @Author: BlahGeek
* @Date:   2014-12-18
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <iostream>
#include "./table_and_index_env.h"

#include "usql/executor/insert.h"

using namespace usql;

class InsertTest: public TableAndIndexEnv {
protected:
    std::vector<rowid_t> do_insert(std::string sql, bool debug = false) {
        SQLStatement stmt(sql);
        if(debug) stmt.setDebug(true);
        EXPECT_EQ(stmt.parse(), 0);
        stmt.normalize();
        stmt.print(std::cerr);
        std::cerr << std::endl;

        std::vector<rowid_t> ret;

        std::vector<std::shared_ptr<TableInfo>> tableinfos{tbinfo0};

        auto exe = std::make_unique<InsertExecutor>(tableinfos, 
                                                    stmt.column_names);
        for(auto & vals: stmt.values)
            ret.push_back(exe->insert(vals));
        return ret;
    }
};

TEST_F(InsertTest, basic_insert) {
    std::string sql("INSERT INTO table0 VALUES (424242, 424243, \"hi\")");
    auto row = do_insert(sql)[0];

    auto vals = table0->find(row);
    EXPECT_EQ(vals[0], LiteralData(424242));

    auto index_rows = t0_c1_index->find(IndexBase::BoundType::INCLUDE, LiteralData(424243),
                                        IndexBase::BoundType::INCLUDE, LiteralData(424243));
    EXPECT_EQ(index_rows.size(), 1);
    EXPECT_NE(index_rows.find(row), index_rows.end());

}

TEST_F(InsertTest, reject_unique) {
    std::string sql("INSERT INTO table0 VALUES (42, -4, \"hi\")");
    ASSERT_ANY_THROW(do_insert(sql));
}

TEST_F(InsertTest, insert_multiple) {
    std::string sql("INSERT INTO table0 (c0, c1, c2) VALUES \n"
                    "(23, 10, \"\"), (23, 11, \"\"), (23, 12, \"\")");
    auto rows = do_insert(sql);
    EXPECT_EQ(rows.size(), 3);
    auto index_rows = t0_c1_index->find(IndexBase::BoundType::INCLUDE, LiteralData(10),
                                        IndexBase::BoundType::INCLUDE, LiteralData(12));
    EXPECT_EQ(index_rows.size(), 3);
}

TEST_F(InsertTest, reject_count) {
    std::string sql("INSERT INTO table0 VALUES (42)");
    ASSERT_ANY_THROW(do_insert(sql));
}

TEST_F(InsertTest, insert_partial) {
    std::string sql("INSERT INTO table0 (c1, c2) VALUES (42, \"hi\")");
    auto row = do_insert(sql)[0];

    auto vals = table0->find(row);
    EXPECT_EQ(vals[0], LiteralData(0));
}

TEST_F(InsertTest, insert_partial_reject_unique) {
    std::string sql("INSERT INTO table0 (c0, c2) VALUES (42, \"hi\")");
    ASSERT_ANY_THROW(do_insert(sql));
}
