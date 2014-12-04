%{
#include <string>
#include "sql_scanner.h"
#include "../datatype/base.h"
#include "../sql_statement.h"

using token = usql::SQLParser::token;

#define yyterminate() return( token::END )
 
/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H
 
%}

%option debug 
%option nodefault 
%option yyclass="SQLScanner" 
%option noyywrap 
%option c++

DIGIT               = ([0-9])
HEX_DIGIT           = ([0-9A-Fa-f])
HEX_INTEGER         = (0[Xx]{HEX_DIGIT}+)
DEC_INTEGER         = ({DIGIT}+)
INTEGER             = ({HEX_INTEGER}|{DEC_INTEGER})
FLOAT               = ({DEC_INTEGER}"."{DEC_INTEGER})
IDENTIFIER          = ([_0-9A-Za-z]*[_A-Za-z][_0-9A-Za-z]*)
SIMPLE_OPERATOR     = ("+"|"-"|"*"|"/"|"%"|"="|"<"|">"|"."|","|";"|"!"|"("|")"|"["|"]"|"{"|"}"|"&"|":")
WHITESPACE          = ([ \t\n]+)

%%

SIMPLE_OPERATOR {return (int)yytext[0]; }

"INT"           return token::INT;
"VARCHAR"       return token::VARCHAR;

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

"CREATE"        return token::CREATE;
"TABLE"         return token::TABLE;

WHITESPACE      {}
[0-9]+          {
                    yylval->intdata = atoi(yytext);
                    return token::INTEGER;
                }
[a-zA-Z0-9]+    {
                    yylval->stringdata = new std::string(yytext);
                    return token::IDENTIFIER;
                }

%%

