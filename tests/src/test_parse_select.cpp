/* 
* @Author: BlahGeek
* @Date:   2014-12-09
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-14
*/

#include <iostream>
#include "./usql/sql_statement.h"
#include "./usql/parser/sql.tab.hpp"
#include "./usql/parser/sql_scanner.h"

#include <string>
#include <gtest/gtest.h>
#include <strstream>

using namespace std;
using namespace usql;

TEST(SQLParserTest, where_statement) {
    std::ostrstream out;
    string sql("SELECT * FROM xxx,yyy,xyz,xxx \n"
               "WHERE (1 = 1 OR 1 > 2) OR (xxx.col1 > 3 AND xxx.col1 <= 10)\n"
               "OR NOT (xxx.col1 != 122 AND (yyy.col2 = 7 OR yyy.col2 = xxx.col3))");
    SQLStatement stmt(sql);
    stmt.parse();
    stmt.print(cout);
    cout << endl;
    stmt.print(out);
    EXPECT_EQ(out.str(), std::string("SELECT *.* FROM xxx, yyy, xyz, xxx "
              "WHERE (((1 = 1) OR (1 > 2)) OR ((xxx.col1 > 3) AND "
              "(xxx.col1 <= 10))) OR (NOT ((xxx.col1 != 122) AND "
              "((yyy.col2 = 7) OR (yyy.col2 = xxx.col3))))"));
}

TEST(SQLParserTest, select_values) {
    std::ostrstream out;
    string sql("select a,c,t1.d from t1,t2 where 1 = 1");
    SQLStatement stmt(sql);
    stmt.parse();
    stmt.print(cout);
    cout << endl;
    stmt.print(out);
    EXPECT_EQ(out.str(), std::string("SELECT .a, .c, t1.d"
              " FROM t1, t2 WHERE 1 = 1"));
}

TEST(SQLParserTest, normalize) {
    std::ostrstream out;
    string sql("select a,c,t1.d from t1,t2 where 1 = 1 and x = d");
    SQLStatement stmt(sql);
    stmt.parse();
    stmt.normalize();
    stmt.print(cout);
    cout << endl;
    stmt.print(out);
    EXPECT_EQ(out.str(), std::string("SELECT t1.a, t1.c, t1.d"
              " FROM t1, t2 WHERE (1 = 1) AND (t1.x = t1.d)"));
}
