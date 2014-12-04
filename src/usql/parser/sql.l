%{
#include <iostream>
#include <string>
#include "sql_scanner.h"
#include "../datatype/base.h"
#include "../sql_statement.h"

using std::cout;
using std::endl;

using token = usql::SQLParser::token;

#define yyterminate() return( token::END )
 
/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H
 
%}

%option debug 
%option yyclass="SQLScanner" 
%option noyywrap 
%option c++

%%

[ \t\n]+        { }

"INT"           { return token::INT; }
"VARCHAR"       { return token::VARCHAR; }

"PRIMARY KEY"   {
                    yylval->column_constraint = usql::SQLStatement::ColumnConstraint::PRIMARY;
                    return token::COLUMN_CONSTRAINT;
                }
"NOT NULL"      {
                    yylval->column_constraint = usql::SQLStatement::ColumnConstraint::NOT_NULL;
                    return token::COLUMN_CONSTRAINT;
                }
"UNIQUE"        {
                    yylval->column_constraint = usql::SQLStatement::ColumnConstraint::UNIQUE;
                    return token::COLUMN_CONSTRAINT;
                }

"CREATE"        { return token::CREATE; }
"TABLE"         { return token::TABLE; }

[0-9]+          {
                    yylval->intdata = atoi(yytext);
                    return token::INTEGER;
                }
[a-zA-Z0-9]+    {
                    yylval->stringdata = new std::string(yytext);
                    return token::IDENTIFIER;
                }

.               { return int(yytext[0]); }

%%

