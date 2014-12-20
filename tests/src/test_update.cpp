/* 
* @Author: BlahGeek
* @Date:   2014-12-20
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-20
*/

#include <iostream>

#include "./table_and_index_env.h"
#include "usql/executor/update.h"

using namespace usql;

class UpdateTest: public TableAndIndexEnv {
protected:
    std::set<rowid_t> do_update(std::string sql, bool debug = false) {
        SQLStatement stmt(sql);
        stmt.setDebug(debug);

        EXPECT_EQ(stmt.parse(), 0);
        stmt.normalize();
        stmt.print(std::cerr);
        std::cerr << std::endl;

        std::vector<std::shared_ptr<TableInfo>> tableinfos { tbinfo0 };
        auto exe = std::make_unique<UpdateExecutor>(tableinfos, stmt.column_names);

        return exe->execute(stmt.values.back(), 
                            std::move(stmt.where_stmt));
    }
};

TEST_F(UpdateTest, basic_update) {
    std::string sql("UPDATE table0 SET c2 = \"WTF\" WHERE\n"
                    "c0 = 0");
    auto rows = do_update(sql);

    EXPECT_EQ(rows.size(), 1);
    EXPECT_NE(rows.find(0), rows.end());

    auto vals = table0->find(0);
    EXPECT_EQ(vals[2], LiteralData("WTF"));
}

TEST_F(UpdateTest, update_not_unique) {
    std::string sql("UPDATE table0 SET c1 = 1");
    ASSERT_ANY_THROW(do_update(sql));

    auto rows = t0_c1_index->find(LiteralData(1));
    EXPECT_EQ(rows.size(), 1);
}
