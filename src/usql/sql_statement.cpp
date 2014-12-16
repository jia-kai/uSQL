/* 
* @Author: BlahGeek
* @Date:   2014-12-03
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-16
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
        case SQLStatement::Type::SELECT:
            stream << "SELECT ";
            for(size_t i = 0 ; i < select_vals.size() ; i += 1) {
                if (i != 0) stream << ", ";
                stream << select_vals[i].first << "." << select_vals[i].second ;
            }
            stream << " FROM ";
            for(size_t i = 0 ; i < table_names.size() ; i += 1) {
                if(i != 0) stream << ", ";
                stream << table_names[i];
            }
            if(where_stmt) {
                stream << " WHERE ";
                where_stmt->print(stream);
            }
            break;
        default:
            stream << "NOT IMPLEMENTED";
            break;
    }
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

void SQLStatement::normalize() {
    if(!where_stmt) {
        where_stmt = std::make_unique<WhereStatement>();
        // 0 = 0
        where_stmt->a = LiteralData(0);
        where_stmt->b = LiteralData(0); 
    }
    where_stmt->normalize();
    
    if(type == SQLStatement::Type::SELECT) {
        for(auto & val: select_vals)
            if(val.first.length() == 0)
                val.first = table_names[0];
        where_stmt->setDefaultTable(table_names[0]);
    }
}
