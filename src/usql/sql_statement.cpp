/* 
* @Author: BlahGeek
* @Date:   2014-12-03
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-04
*/

#include "./sql_statement.h"
#include "./datatype/base.h"
#include <strstream>
#include <iostream>
#include "./common.h"

#include "./parser/sql.tab.hpp"
#include "./parser/sql_scanner.h"

using namespace usql;
using namespace std;

ostream & SQLStatement::print(ostream & stream) {
    stream << "SQL: " << origin << endl;
    stream << "SQL Type:\t" << int(type) << endl;
    stream << "Identifier:\t" << identifier << endl;
    stream << "Column definitions:" << endl;
    for(auto & column: columns) {
        stream << "\t" << column.first 
            << "\t: " << column.second->type_name() << "\t";
        for(auto & cons: column_constraints[column.first]) {
            stream << ", " << int(cons);
        }
        stream << endl;
    }
    // TODO: print values
    return stream;
}

SQLStatement::SQLStatement(std::string sql, bool debug) {
    origin = sql;
    istrstream st(sql.c_str());
    scanner = std::unique_ptr<SQLScanner>(new SQLScanner(&st));
    parser = std::unique_ptr<SQLParser>(new SQLParser(*scanner, *this));
    parser->set_debug_level(debug);
    scanner->setDebug(debug);
    auto ret = parser->parse();
    if(ret != 0)
        throw std::string("parse sql failed");
}
