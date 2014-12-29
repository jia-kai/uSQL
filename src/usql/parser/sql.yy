%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 
%define api.namespace {usql}
%define parser_class_name {SQLParser}

%code requires{
    #include "./sql_statement.h"
    #include "../common.h"
    #include "../datatype/base.h"
    namespace usql {
        class SQLStatement;
        class SQLScanner;
    }
}

%lex-param   { SQLScanner  &scanner  }
%parse-param { SQLScanner  &scanner  }
 
%lex-param   { SQLStatement  &driver  }
%parse-param { SQLStatement  &driver  }

%code{
    #include <iostream>
    #include <cstdlib>
    #include <fstream>
    #include "../common.h"
    #include "../datatype/base.h"
    #include "../datatype/int.h"
    #include "../datatype/string.h"
    #include "./sql_statement.h"
    #include "./where_statement.h"

    /* this is silly, but I can't figure out a way around */
    static int yylex(usql::SQLParser::semantic_type *yylval,
        usql::SQLScanner  &scanner,
        usql::SQLStatement   &driver);
}

/* token types */
%union {
    usql::DataTypeBase * datatype;
    usql::SQLStatement::ColumnConstraint column_constraint;
    usql::LiteralData * literal_data;
    std::vector<usql::LiteralData> * literal_datas;
    usql::WhereStatement::WhereStatementOperator where_op;
    usql::WhereStatement * where_stmt;
    ColumnAndTableName * column_and_table;
    int64_t intdata;
    std::string * stringdata;
}
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <datatype>
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <stringdata>
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <literal_data>
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <literal_datas>
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <where_stmt>
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <column_and_table>

%token <stringdata>    IDENTIFIER
%token <intdata>       INTEGER
%token <stringdata>    LITERAL_STRING
%token <column_constraint>  COLUMN_CONSTRAINT

%type <literal_data> literal_data
%type <literal_datas> literal_datas
%type <where_stmt> where_stmt
%type <column_and_table> column_and_table column_and_table_or_expand

%type <datatype>              datatype
%type <stringdata>            column_def

%token CREATE TABLE DATABASE DROP SHOW USE
%token FROM SELECT WHERE
%token INSERT INTO VALUES
%token DELETE
%token UPDATE SET
%token INDEX ON
%token TABLES

%token AND OR NOT

%left AND OR
%right NOT

%token <where_op>     WHERE_OP
%left WHERE_OP

%token INT VARCHAR
%token NULL_

%left ','


%token END 0 "EOF"

%%

sql_statement       : CREATE TABLE IDENTIFIER '(' column_defs ')' END {
                         driver.type = usql::SQLStatement::Type::CREATE_TB;
                         driver.table_names.push_back(*($3));
                      }
                    | SELECT column_names FROM tables where_or_empty END {
                        driver.type = usql::SQLStatement::Type::SELECT;
                    }
                    | INSERT INTO IDENTIFIER insert_columns VALUES insert_values END {
                        driver.type = usql::SQLStatement::Type::INSERT;
                        driver.table_names.push_back(*($3));
                    }
                    | DELETE FROM IDENTIFIER where_or_empty END {
                        driver.type = usql::SQLStatement::Type::DELETE;
                        driver.table_names.push_back(*($3));
                    }
                    | UPDATE IDENTIFIER SET update_values where_or_empty END {
                        driver.type = usql::SQLStatement::Type::UPDATE;
                        driver.table_names.push_back(*($2));
                    }
                    | CREATE INDEX ON IDENTIFIER '(' column_names ')' END {
                        driver.type = usql::SQLStatement::Type::CREATE_IDX;
                        driver.table_names.push_back(*($4));
                    }
                    | SHOW TABLES {
                        driver.type = usql::SQLStatement::Type::SHOW_TBS;
                    }
                    | CREATE DATABASE IDENTIFIER {
                        driver.type = usql::SQLStatement::Type::CREATE_DB;
                        driver.database_name = *($3);
                    }
                    | DROP DATABASE IDENTIFIER {
                        driver.type = usql::SQLStatement::Type::DROP_DB;
                        driver.database_name = *($3);
                    }
                    | USE IDENTIFIER {
                        driver.type = usql::SQLStatement::Type::USE_DB;
                        driver.database_name = *($2);
                    }
                    ;

update_values       : column_and_table WHERE_OP literal_data {
                        if($2 == usql::WhereStatement::WhereStatementOperator::EQ) {
                            driver.column_names.push_back(*($1));
                            driver.values.emplace_back();
                            driver.values[0].push_back(*($3));
                        }
                    }
                    | update_values ',' column_and_table WHERE_OP literal_data {
                        if($4 == usql::WhereStatement::WhereStatementOperator::EQ) {
                            driver.column_names.push_back(*($3));
                            driver.values[0].push_back(*($5));
                        }
                    }
                    ;

where_or_empty      : { driver.where_stmt = nullptr; }
                    | WHERE where_stmt {
                        driver.where_stmt = std::unique_ptr<usql::WhereStatement>($2); 
                        $2 = nullptr;
                    }
                    ;

insert_columns      : | '(' column_names ')' ;

insert_values       : '(' literal_datas ')' {
                        driver.values.push_back(*($2));
                    }
                    | insert_values ',' '(' literal_datas ')' {
                        driver.values.push_back(*($4));
                    }
                    ;

column_names        : column_and_table_or_expand { driver.column_names.push_back(*($1)); }
                    | column_names ',' column_and_table_or_expand { driver.column_names.push_back(*($3)); }
                    ;

tables              : IDENTIFIER { driver.table_names.push_back(*($1)); }
                    | tables ',' IDENTIFIER { driver.table_names.push_back(*($3)); }
                    ;

where_stmt          : literal_data WHERE_OP literal_data {
                        $$ = new usql::WhereStatement;
                        $$->op = $2;
                        $$->a = *($1); $$->b = *($3);
                      }
                    | column_and_table WHERE_OP literal_data {
                        $$ = new usql::WhereStatement;
                        $$->op = $2;
                        $$->a_is_literal = false;
                        $$->na = *($1);
                        $$->b = *($3);
                    }
                    | literal_data WHERE_OP column_and_table {
                        $$ = new usql::WhereStatement;
                        $$->op = $2; $$->a = *($1);
                        $$->b_is_literal = false;
                        $$->nb = *($3);
                    }
                    | column_and_table WHERE_OP column_and_table {
                        $$ = new usql::WhereStatement;
                        $$->op = $2;
                        $$->a_is_literal = $$->b_is_literal = false;
                        $$->na = *($1);
                        $$->nb = *($3);
                    } 
                    | '(' where_stmt ')' {
                        $$ = $2; $2 = nullptr;
                    }
                    | where_stmt AND where_stmt {
                        $$ = new usql::WhereStatement;
                        $$->type = usql::WhereStatement::WhereStatementType::AND;
                        $$->children.push_back(std::unique_ptr<usql::WhereStatement>($1));
                        $$->children.push_back(std::unique_ptr<usql::WhereStatement>($3));
                        $1 = $3 = nullptr;
                    }
                    | where_stmt OR where_stmt {
                        $$ = new usql::WhereStatement;
                        $$->type = usql::WhereStatement::WhereStatementType::OR;
                        $$->children.push_back(std::unique_ptr<usql::WhereStatement>($1));
                        $$->children.push_back(std::unique_ptr<usql::WhereStatement>($3));
                        $1 = $3 = nullptr;
                    }
                    | NOT where_stmt {
                        $$ = new usql::WhereStatement;
                        $$->type = usql::WhereStatement::WhereStatementType::NOT;
                        $$->children.push_back(std::unique_ptr<usql::WhereStatement>($2));
                        $2 = nullptr;
                    }
                    ;

column_and_table_or_expand : column_and_table {$$ = $1; $1 = nullptr; }
                    | IDENTIFIER '.' '*' {$$ = new ColumnAndTableName(*($1), "*"); }
                    | '*' {$$ = new ColumnAndTableName("*", "*"); }
                    ;

column_and_table    : IDENTIFIER {$$ = new std::pair<std::string, std::string>("", *($1));}
                    | IDENTIFIER '.' IDENTIFIER {
                        $$ = new std::pair<std::string, std::string>(*($1), *($3));
                    }
                    ;


column_defs         : column_def 
                    | seperate_constraint
                    | column_defs ',' column_def
                    | column_defs ',' seperate_constraint
                    | column_defs ','
                    ;

column_def          : IDENTIFIER datatype {
                         driver.column_defs.emplace_back(*($1),
                             std::shared_ptr<usql::DataTypeBase>($2));
                         $2 = nullptr;
                         if(!$$)
                             $$ = new std::string(*($1));
                         else
                             $$->assign(*($1));
                      }
                    | column_def COLUMN_CONSTRAINT {
                         driver.column_constraints[*($1)].insert($2);
                         if(!$$)
                             $$ = new std::string(*($1));
                         else
                             $$->assign(*($1));
                    }
                    ;

seperate_constraint : COLUMN_CONSTRAINT '(' IDENTIFIER ')' {
                         driver.column_constraints[*($3)].insert($1);
                      }
                    ;

datatype            : INT {$$ = new usql::IntDataType(); }
                    | INT '(' INTEGER ')' {$$ = new usql::IntDataType(); /* ignore INT size */}
                    | VARCHAR '(' INTEGER ')' {$$ = new usql::StringDataType($3);}
                    ;

literal_data        : LITERAL_STRING { $$ = new usql::LiteralData(*($1)); }
                    | INTEGER        { $$ = new usql::LiteralData($1); }
                    | NULL_          { $$ = new usql::LiteralData(""); /* NULL is only for string... FIXME */}
                    ;

literal_datas       : literal_data {
                        $$ = new std::vector<usql::LiteralData>();
                        $$->push_back(*($1));
                    }
                    | literal_datas ',' literal_data {
                        $1->push_back(*($3));
                        $$ = $1;
                        $1 = nullptr;
                    }
                    ;


%%

void usql::SQLParser::error(const std::string & msg) {
    std::cerr << "Error: " << msg << std::endl;
}

/* include for access to scanner.yylex */
#include "sql_scanner.h"
static int yylex(usql::SQLParser::semantic_type *yylval,
    usql::SQLScanner  &scanner,
    usql::SQLStatement   &driver){
        return scanner.yylex(yylval);
}
