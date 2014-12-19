/* 
* @Author: BlahGeek
* @Date:   2014-12-15
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-19
*/

#include <iostream>
#include "./table_and_index_env.h"

#include "usql/executor/select.h"

using namespace usql;

class SelectTest: public TableAndIndexEnv {
protected:
    std::vector<std::vector<LiteralData>>
            select(const std::vector<std::shared_ptr<TableInfo>> tableinfos,
                   std::vector<ColumnAndTableName> dests,
                   const std::unique_ptr<WhereStatement> & where) {
        std::vector<std::vector<LiteralData>> ret;
        auto exe = std::make_unique<SelectExecutor>(tableinfos, dests);
        exe->find(where, [&](rowid_t rowid, const std::vector<LiteralData> & vals) -> bool {
            ret.push_back(vals);
            return false;
        });
        return ret;
    }

    std::vector<std::vector<LiteralData>>
            parse_and_select(std::string sql, bool debug = false) {
        SQLStatement stmt(sql);
        if(debug) stmt.setDebug(true);
        EXPECT_EQ(stmt.parse(), 0);
        stmt.normalize();

        std::vector<std::shared_ptr<TableInfo>> tableinfos;
        for(auto & tbname: stmt.table_names) {
            if(tbname == "table0") tableinfos.push_back(tbinfo0);
            if(tbname == "table1") tableinfos.push_back(tbinfo1);
        }
        return select(tableinfos, stmt.column_names, stmt.where_stmt);
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
    auto vals = select({tbinfo0}, dests, where_stmt);

    int i = 0;
    EXPECT_EQ(vals.size(), 10000);
    for(auto & row: vals) {
        EXPECT_EQ(row[0].int_v, i++);
    }
}

TEST_F(SelectTest, single_table_single_column_no_index) {
    auto where_stmt = std::make_unique<WhereStatement>();
    where_stmt->a = 10;
    where_stmt->b_is_literal = false;
    where_stmt->op = WhereStatement::WhereStatementOperator::GT;
    where_stmt->nb = ColumnAndTableName("table0", "c0");
    where_stmt->normalize();

    std::vector<ColumnAndTableName> dests = {
        ColumnAndTableName("table0", "c0"),
        ColumnAndTableName("table0", "c2")
    };
    auto vals = select({tbinfo0}, dests, where_stmt);

    EXPECT_EQ(vals.size(), 10);
    for(int i = 0 ; i < 10 ; i += 1)
        EXPECT_EQ(vals[i][0].int_v, i);
}

TEST_F(SelectTest, single_table_single_column_index) {
    auto where_stmt = std::make_unique<WhereStatement>();
    where_stmt->a_is_literal = false;
    where_stmt->na = ColumnAndTableName("table0", "c1");
    where_stmt->op = WhereStatement::WhereStatementOperator::GE;
    where_stmt->b = -3;
    where_stmt->normalize();

    std::vector<ColumnAndTableName> dests = {
        ColumnAndTableName("table0", "c0")
    };
    auto vals = select({tbinfo0}, dests, where_stmt);
    EXPECT_EQ(vals.size(), 4);
    for(int i = 0 ; i < 4 ; i += 1)
        EXPECT_EQ(vals[i][0].int_v, i);
}

TEST_F(SelectTest, single_table_index_multiple_column) {
    std::string sql("SELECT c0,c2 FROM table0 WHERE\n"
                    "c1 >= -42 and c1 <= 0\n"
                    "and c0 < 10 and 2 > 1");
    auto vals = parse_and_select(sql);

    EXPECT_EQ(vals.size(), 10);
}

TEST_F(SelectTest, single_table_index_multiple_column_string) {
    std::string sql("SELECT c0,c2 FROM table0 WHERE\n"
                    "c1 >= -42 and c1 <= 0\n"
                    "and c0 < 10 and 2 > 1\n"
                    "and c2 = \"No. 1\"");
    auto vals = parse_and_select(sql);

    EXPECT_EQ(vals.size(), 1);
    EXPECT_EQ(vals[0][1].string_v, "No. 1");
}

TEST_F(SelectTest, dual_table_dual_index) {
    std::string sql("SELECT table0.c1, table1.c0\n"
                    "FROM table0, table1 WHERE\n"
                    "table0.c1 >= -9 and\n"
                    "table1.c0 = \"10\"");
    auto vals = parse_and_select(sql);
    EXPECT_EQ(vals.size(), 10);
}

TEST_F(SelectTest, dual_table_and_expand) {
    std::string sql("SELECT table0.*, *\n"
                    "FROM table1, table0 WHERE\n"
                    "1 = 1 and \n"
                    "table1.c0 = \"10\"");
    auto vals = parse_and_select(sql);
    EXPECT_EQ(vals.size(), 10000);
}

TEST_F(SelectTest, single_table_no_where_stmt) {
    std::string sql("SELECT *\n"
                    "FROM table0\n");
    auto vals = parse_and_select(sql);
    EXPECT_EQ(vals.size(), 10000);
}

TEST_F(SelectTest, single_table_multiple_match) {
    std::string sql("SELECT *\n"
                    "FROM table1 WHERE\n"
                    "c1 = 0");
    auto vals = parse_and_select(sql);
    EXPECT_EQ(vals.size(), 10);
}

TEST_F(SelectTest, single_table_string_index) {
    std::string sql("SELECT *\n"
                    "FROM table1 WHERE\n"
                    "not c0 = \"0\"");
    auto vals = parse_and_select(sql);
    EXPECT_EQ(vals.size(), 999);
}

TEST_F(SelectTest, dual_table_ultimate) {
    std::string sql("SELECT table0.c1, table1.*\n"
                    "FROM table0, table1 WHERE\n"
                    "(table0.c1 >= -99 or table0.c1 >= 42) and\n"
                    "(table1.c1 = 0 and not table1.c0 = \"900\") and\n"
                    " table0.c0 != table1.c1");
    auto vals = parse_and_select(sql);
    EXPECT_EQ(vals.size(), 99*9);
}
