%skeleton "lalr1.cc"
%require  "3.0"
%debug 
%defines 
%define api.namespace {usql}
%define parser_class_name {SQLParser}

%code requires{
    #include "../sql_statement.h"
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
    #include "../sql_statement.h"

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
    usql::WhereStatement::WhereStatementOperator where_op;
    usql::WhereStatement * where_stmt;
    int64_t intdata;
    std::string * stringdata;
}
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <datatype>
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <stringdata>
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <literal_data>
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <where_stmt>

%token <stringdata>    IDENTIFIER
%token <intdata>       INTEGER
%token <stringdata>    LITERAL_STRING
%token <column_constraint>  COLUMN_CONSTRAINT

%type <literal_data> literal_data
%type <where_stmt> where_stmt

%type <datatype>              datatype
%type <stringdata>            column_def

%token CREATE TABLE DATABASE DROP SHOW USE
%token FROM SELECT WHERE

%token AND OR NOT

%left AND OR
%right NOT

%token <where_op>     WHERE_OP
%left WHERE_OP

%token INT VARCHAR


%token END 0 "EOF"

%%

sql_statement       : CREATE TABLE IDENTIFIER '(' column_defs ')' END {
                         driver.type = usql::SQLStatement::Type::CREATE_TB;
                         driver.table_names.push_back(*($3));
                      }
                    | SELECT select_vals FROM tables WHERE where_stmt END {
                        driver.type = usql::SQLStatement::Type::SELECT;
                        driver.where_stmt = std::unique_ptr<usql::WhereStatement>($6); $6 = nullptr;
                    }
                    ;

select_vals         : '*' ;

tables              : IDENTIFIER { driver.table_names.push_back(*($1)); }
                    | tables ',' IDENTIFIER { driver.table_names.push_back(*($3)); }
                    ;

where_stmt          : literal_data WHERE_OP literal_data {
                        $$ = new usql::WhereStatement;
                        $$->op = $2;
                        $$->a = *($1); $$->b = *($3);
                      }
                    | IDENTIFIER '.' IDENTIFIER WHERE_OP literal_data {
                        $$ = new usql::WhereStatement;
                        $$->op = $4;
                        $$->a_is_literal = false;
                        $$->na = std::pair<std::string, std::string>(*($1), *($3));
                        $$->b = *($5);
                    }
                    | literal_data WHERE_OP IDENTIFIER '.' IDENTIFIER {
                        $$ = new usql::WhereStatement;
                        $$->op = $2; $$->a = *($1);
                        $$->b_is_literal = false;
                        $$->nb = std::pair<std::string, std::string>(*($3), *($5));
                    }
                    | IDENTIFIER '.' IDENTIFIER WHERE_OP IDENTIFIER '.' IDENTIFIER {
                        $$ = new usql::WhereStatement;
                        $$->op = $4;
                        $$->a_is_literal = $$->b_is_literal = false;
                        $$->na = std::pair<std::string, std::string>(*($1), *($3));
                        $$->nb = std::pair<std::string, std::string>(*($5), *($7));
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
                         if($$) delete($$);
                         $$ = new std::string(*($1));
                      }
                    | column_def COLUMN_CONSTRAINT {
                         driver.column_constraints[*($1)].insert($2);
                         if($$) delete($$);
                         $$ = new std::string(*($1));
                    }
                    ;

seperate_constraint : COLUMN_CONSTRAINT '(' IDENTIFIER ')' {
                         driver.column_constraints[*($3)].insert($1);
                      }
                    ;

datatype            : INT {$$ = new usql::IntDataType(); }
                    | VARCHAR '(' INTEGER ')' {$$ = new usql::StringDataType($3);}
                    ;

literal_data        : LITERAL_STRING { $$ = new usql::LiteralData(*($1)); }
                    | INTEGER        { $$ = new usql::LiteralData($1); }
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
