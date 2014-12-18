/* 
* @Author: BlahGeek
* @Date:   2014-12-19
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-19
*/

#include <iostream>
#include "./table_and_index_env.h"

#include "usql/delete_executor.h"

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

        auto exe = std::make_unique<DeleteExecutor>(tbinfo0);
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
