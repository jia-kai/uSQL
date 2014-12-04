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

TEST(SQLParserTest, TableSchema) {
    string sql;
    cin >> sql;
    SQLStatement stmt(sql);
    stmt.print(cout);
}

