/* 
* @Author: BlahGeek
* @Date:   2014-12-15
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-15
*/

#include <iostream>
#include "./table_and_index_env.h"

#include "usql/select_executor.h"

using namespace usql;

class SelectTest: public TableAndIndexEnv {
protected:
    std::unique_ptr<SelectExecutor> t0_selector;
    std::unique_ptr<SelectExecutor> dual_selector;

    void SetUp() override {
        TableAndIndexEnv::SetUp();
        t0_selector = std::make_unique<SelectExecutor>();
        dual_selector = std::make_unique<SelectExecutor>();

        t0_selector->addTable(std::string("table0"), table0);
        t0_selector->addIndex(ColumnAndTableName("table0", "c1"),
                              t0_c1_index);

        dual_selector->addTable(std::string("table0"), table0);
        dual_selector->addTable(std::string("table1"), table1);
        dual_selector->addIndex(ColumnAndTableName("table0", "c1"),
                                t0_c1_index);
        dual_selector->addIndex(ColumnAndTableName("table1", "c0"),
                                t1_c0_index);
    }

    std::vector<std::vector<LiteralData>> 
            select(const std::unique_ptr<SelectExecutor> & selector,
                   std::vector<ColumnAndTableName> dests,
                   const std::unique_ptr<WhereStatement> & where) {
        std::vector<std::vector<LiteralData>> ret;
        selector->execute(dests, where,
                          [&](const std::vector<LiteralData> & vals){
                            ret.push_back(vals);
                          });
        return ret;
    }
};

TEST_F(SelectTest, single_table_all_match) {
    auto where_stmt = std::make_unique<WhereStatement>();
    where_stmt->a = where_stmt->b = 1;
    where_stmt->normalize();

    std::vector<ColumnAndTableName> dests = {
        ColumnAndTableName("table0", "c0"),
        ColumnAndTableName("table0", "c1"),
        ColumnAndTableName("table0", "c2")
    };
    auto vals = select(t0_selector, dests, where_stmt);

    int i = 0;
    EXPECT_EQ(vals.size(), 10000);
    for(auto & row: vals) {
        EXPECT_EQ(row[0].int_v, i++);
    }
}
