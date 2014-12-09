/* 
* @Author: BlahGeek
* @Date:   2014-12-09
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-09
*/

#include <iostream>
#include "./usql/sql_statement.h"
#include "./usql/parser/sql.tab.hpp"
#include "./usql/parser/sql_scanner.h"

#include <string>
#include <gtest/gtest.h>

using namespace std;
using namespace usql;

TEST(SQLParserTest, where_statement) {
    string sql("SELECT * FROM xxx WHERE 1 = 1");
    SQLStatement stmt(sql);
    stmt.parse();
    stmt.print(cout);
    // EXPECT_TRUE(stmt.type == SQLStatement::Type::CREATE_TB);
    // EXPECT_TRUE(stmt.table_names[0] == "xxx");
    // EXPECT_TRUE(stmt.column_defs.size() == 1);
}
