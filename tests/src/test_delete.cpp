/* 
* @Author: BlahGeek
* @Date:   2014-12-19
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-19
*/

#include <iostream>
#include "./table_and_index_env.h"

#include "usql/executor/delete.h"

using namespace usql;

class DeleteTest: public TableAndIndexEnv {
protected:
    std::set<rowid_t> do_delete(std::string sql, bool debug = false) {
        SQLStatement stmt(sql);
        stmt.setDebug(debug);

        EXPECT_EQ(stmt.parse(), 0);
        stmt.normalize();
        stmt.print(std::cerr);
        std::cerr << std::endl;

        std::vector<std::shared_ptr<TableInfo>> tableinfos { tbinfo0 };

        auto exe = std::make_unique<DeleteExecutor>(tableinfos);
        return exe->execute(std::move(stmt.where_stmt));
    }
};

TEST_F(DeleteTest, simple_delete_indexed) {
    std::string sql("DELETE FROM table0 WHERE\n"
                    "c1 < -1");
    auto ret = do_delete(sql);

    auto tmp = t0_c1_index->find(IndexBase::BoundType::DISABLE, LiteralData(),
                      IndexBase::BoundType::DISABLE, LiteralData());
    EXPECT_EQ(tmp.size(), 2);
}

TEST_F(DeleteTest, simple_delete_no_indexed) {
    std::string sql("DELETE FROM table0 WHERE\n"
                    "c2 != \"No. 4242\"");
    auto ret = do_delete(sql);

    auto tmp = t0_c1_index->find(IndexBase::BoundType::DISABLE, LiteralData(),
                      IndexBase::BoundType::DISABLE, LiteralData());
    EXPECT_EQ(tmp.size(), 1);
}

TEST_F(DeleteTest, simple_delete_mixed) {
    std::string sql("DELETE FROM table0 WHERE\n"
                    "c1 < -9 or c0 > 3");
    auto ret = do_delete(sql);

    auto tmp = t0_c1_index->find(IndexBase::BoundType::DISABLE, LiteralData(),
                      IndexBase::BoundType::DISABLE, LiteralData());
    EXPECT_EQ(tmp.size(), 4);
}

TEST_F(DeleteTest, delete_all) {
    std::string sql("DELETE FROM table0\n");
    auto ret = do_delete(sql);

    auto tmp = t0_c1_index->find(IndexBase::BoundType::DISABLE, LiteralData(),
                      IndexBase::BoundType::DISABLE, LiteralData());
    EXPECT_EQ(tmp.size(), 0);
}

TEST_F(DeleteTest, delete_none) {
    std::string sql("DELETE FROM table0 where 1 = 0\n");
    auto ret = do_delete(sql);

    auto tmp = t0_c1_index->find(IndexBase::BoundType::DISABLE, LiteralData(),
                      IndexBase::BoundType::DISABLE, LiteralData());
    EXPECT_EQ(tmp.size(), 10000);
}
