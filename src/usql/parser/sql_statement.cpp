/* 
* @Author: BlahGeek
* @Date:   2014-12-03
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include "./sql_statement.h"
#include "../datatype/base.h"
#include <sstream>
#include <iostream>
#include "../common.h"

#include "./sql.tab.hpp"
#include "./sql_scanner.h"

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
        case SQLStatement::Type::CREATE_IDX:
            stream << "CREATE INDEX ON " << table_names[0] << " (";
            for(size_t i = 0 ; i < column_names.size() ; i += 1) {
                if(i != 0) stream << ", ";
                stream << column_names[i].second;
            }
            stream << ")";
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
                        stream << " PRIMARY KEY";
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
        case SQLStatement::Type::SELECT:
            stream << "SELECT ";
            for(size_t i = 0 ; i < column_names.size() ; i += 1) {
                if (i != 0) stream << ", ";
                stream << column_names[i].first << "." << column_names[i].second ;
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
        case SQLStatement::Type::INSERT:
            stream << "INSERT INTO " << table_names[0] << " ";
            if(!column_names.empty()) {
                stream << "(";
                for(size_t i = 0 ; i < column_names.size() ; i += 1){
                    if(i != 0) stream << ", ";
                    stream << column_names[i].second;
                }
                stream << ") ";
            }
            stream << "VALUES ";
            for(size_t i = 0 ; i < values.size() ; i += 1) {
                if(i != 0) stream << ", ";
                stream << "(";
                for(size_t j = 0 ; j < values[i].size() ; j += 1) {
                    if(j != 0) stream << ", ";
                    values[i][j].print(stream);
                }
                stream << ")";
            }
            break;
        case SQLStatement::Type::DELETE:
            stream << "DELETE FROM " << table_names[0];
            if(where_stmt) {
                stream << " WHERE ";
                where_stmt->print(stream);
            }
            break;
        case SQLStatement::Type::UPDATE:
            stream << "UPDATE " << table_names[0] << " SET ";
            for(size_t i = 0 ; i < column_names.size() ; i += 1) {
                if(i != 0) stream << ", ";
                stream << column_names[i].second << " = ";
                values[0][i].print(stream);
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
    st = std::make_unique<istringstream>(origin.c_str());
    scanner.reset(new SQLScanner{st.get()});
    parser.reset(new SQLParser{*scanner, *this});
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

    for(auto & val: column_names)
        if(val.first.length() == 0)
            val.first = table_names[0];
    where_stmt->setDefaultTable(table_names[0]);
}

void SQLStatement::SQLScannerDeleter::operator () (SQLScanner *ptr) {
    delete ptr;
}

void SQLStatement::SQLParserDeleter::operator () (SQLParser *ptr) {
    delete ptr;
}

