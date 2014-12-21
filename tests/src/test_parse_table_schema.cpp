/* 
* @Author: BlahGeek
* @Date:   2014-12-03
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <iostream>
#include "./usql/parser/sql_statement.h"

#include <string>
#include <gtest/gtest.h>

using namespace std;
using namespace usql;

TEST(SQLParserTest, TableSchema0) {
    string sql("   CREATE TABLE xxx \t(xxx INT)\n\n");
    SQLStatement stmt(sql);
    stmt.parse();
    stmt.print(cout);
    cout << endl;
    EXPECT_TRUE(stmt.type == SQLStatement::Type::CREATE_TB);
    EXPECT_TRUE(stmt.table_names[0] == "xxx");
    EXPECT_TRUE(stmt.column_defs.size() == 1);
}

TEST(SQLParserTest, TableSchema1) {
    string sql("   CREATE TABLE abc123 (col0 INT PRIMARY KEY,\n"
        "col1 INT UNIQUE NOT NULL, col2 VARCHAR(3), NOT NULL(col2), \t)\n\n");
    SQLStatement stmt(sql);
    stmt.parse();
    stmt.print(cout);
    cout << endl;
    EXPECT_TRUE(stmt.type == SQLStatement::Type::CREATE_TB);
    EXPECT_TRUE(stmt.table_names[0] == "abc123");
    EXPECT_TRUE(stmt.column_defs[2].second->type_name() == "VARCHAR(3)");
    EXPECT_TRUE(stmt.column_constraints["col1"].size() == 2);
}

TEST(SQLParserTest, TableSchema2) {
    string sql("   CREATE TABLE abc123 (col0 INT PRIMARY KEY,\n"
        "col1 INT UNIQUE NOT NULL, col2 VARCHAR(), NOT NULL(col2))\n\n");
    SQLStatement stmt(sql);
    EXPECT_NE(stmt.parse(), 0);
}
