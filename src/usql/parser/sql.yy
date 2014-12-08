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
    int64_t intdata;
    std::string * stringdata;
}
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <datatype>
%destructor { if ($$)  { delete ($$); ($$) = nullptr; } } <stringdata>

%token <stringdata>    IDENTIFIER
%token <intdata>       INTEGER
%token <column_constraint>  COLUMN_CONSTRAINT

%type <datatype>              datatype
%type <stringdata>            column_def

%token CREATE TABLE DATABASE DROP SHOW USE
%token FROM SELECT WHERE
%token INT VARCHAR


%token END 0 "EOF"

%%

sql_statement       : CREATE TABLE IDENTIFIER '(' column_defs ')' END {
                         driver.type = usql::SQLStatement::Type::CREATE_TB;
                         driver.table_names.push_back(*($3));
                      }
                    | SELECT select_vals FROM tables WHERE where_statement END {
                        driver.type = usql::SQLStatement::Type::SELECT;
                    }
                    ;

select_vals         : '*' ;

tables              : IDENTIFIER { driver.table_names.push_back(*($1)); }
                    | tables ',' IDENTIFIER { driver.table_names.push_back(*($3)); }
                    ;
where_statement     : ;

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


%%

void usql::SQLParser::error(const std::string & msg) {
//    std::cerr << "Error: " << msg << std::endl;
}

/* include for access to scanner.yylex */
#include "sql_scanner.h"
static int yylex(usql::SQLParser::semantic_type *yylval,
    usql::SQLScanner  &scanner,
    usql::SQLStatement   &driver){
        return scanner.yylex(yylval);
}
