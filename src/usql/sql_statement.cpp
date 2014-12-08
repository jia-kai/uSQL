/* 
* @Author: BlahGeek
* @Date:   2014-12-03
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-08
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
    bool first_enter = true;
    stream << "Original: " << origin << endl;
    stream << "Parsed:   " ;
    switch(type) {
        case SQLStatement::Type::CREATE_DB:
            stream << "CREATE DATABASE " << database_name;
            break;
        case SQLStatement::Type::DROP_DB:
            stream << "DROP DATABASE " << database_name;
            break;
        case SQLStatement::Type::USE_DB:
            stream << "USE " << database_name;
            break;
        case SQLStatement::Type::SHOW_TBS:
            stream << "SHOW TABLES" ;
            break;
        case SQLStatement::Type::CREATE_TB:
            stream << "CREATE TABLE " << table_names[0] << " (";
            first_enter = true;
            for(auto & column: column_defs) {
                if(!first_enter)
                    stream << ", ";
                first_enter = false;

                stream << column.first << " " << column.second->type_name();
                for(auto & cons: column_constraints[column.first]) {
                    if(cons == SQLStatement::ColumnConstraint::PRIMARY)
                        stream << " PRIMARY";
                    else if(cons == SQLStatement::ColumnConstraint::NOT_NULL)
                        stream << " NOT NULL";
                    else if(cons == SQLStatement::ColumnConstraint::UNIQUE)
                        stream << " UNIQUE";
                    else stream << " UNKNOWN";
                }
            }
            stream << ")";
            break;
        case SQLStatement::Type::DROP_TB:
            stream << "DROP TABLE " << table_names[0] ;
            break;
        case SQLStatement::Type::DESC_TB:
            stream << "DESCRIBE TABLE " << table_names[0];
            break;
        default:
            stream << "NOT IMPLEMENTED";
            break;
    }
    stream << endl;
    return stream;
}

SQLStatement::SQLStatement(std::string sql) {
    origin = sql;
    st = std::unique_ptr<istrstream>(new istrstream(origin.c_str()));
    scanner = std::unique_ptr<SQLScanner>(new SQLScanner(st.get()));
    parser = std::unique_ptr<SQLParser>(new SQLParser(*scanner, *this));
}

void SQLStatement::setDebug(bool enable) {
    parser->set_debug_level(enable);
    scanner->setDebug(enable);
}

int SQLStatement::parse(){
    return parser->parse();
}
