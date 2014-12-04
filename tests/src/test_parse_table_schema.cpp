/* 
* @Author: BlahGeek
* @Date:   2014-12-03
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-04
*/

#include <iostream>
#include "./usql/sql_statement.h"
#include "./usql/parser/sql.tab.hpp"
#include "./usql/parser/sql_scanner.h"

#include <string>
#include <gtest/gtest.h>

using namespace std;
using namespace usql;

TEST(SQLParserTest, TableSchema0) {
    string sql("   CREATE TABLE xxx \t(xxx INT)\n\n");
    SQLStatement stmt(sql, false);
    stmt.print(cout);
    EXPECT_TRUE(stmt.type == SQLStatement::Type::CREATE_TB);
    EXPECT_TRUE(stmt.identifier == "xxx");
    EXPECT_TRUE(stmt.columns.size() == 1);
}

TEST(SQLParserTest, TableSchema1) {
    string sql("   CREATE TABLE abc123 (col0 INT PRIMARY KEY,\n"
        "col1 INT UNIQUE NOT NULL, col2 VARCHAR(3), NOT NULL(col2))\n\n");
    SQLStatement stmt(sql, false);
    stmt.print(cout);
    EXPECT_TRUE(stmt.type == SQLStatement::Type::CREATE_TB);
    EXPECT_TRUE(stmt.identifier == "abc123");
    EXPECT_TRUE(stmt.columns[2].second->type_name() == "VARCHAR(3)");
    EXPECT_TRUE(stmt.column_constraints["col1"].size() == 2);
}

TEST(SQLParserTest, TableSchema2) {
    string sql("   CREATE TABLE abc123 (col0 INT PRIMARY KEY,\n"
        "col1 INT UNIQUE NOT NULL, col2 VARCHAR(), NOT NULL(col2))\n\n");
    SQLStatement * stmt = nullptr;
    EXPECT_THROW(stmt = new SQLStatement(sql), string);
}
