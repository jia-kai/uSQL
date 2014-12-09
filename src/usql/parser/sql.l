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

"SELECT"        { return token::SELECT; }
"FROM"          { return token::FROM; }
"WHERE"         { return token::WHERE; }

"AND"           { return token::AND; }
"OR"            { return token::OR; }
"NOT"           { return token::NOT; }

">="            { yylval->where_op = usql::WhereStatement::WhereStatementOperator::GE; return token::WHERE_OP; }
"<="            { yylval->where_op = usql::WhereStatement::WhereStatementOperator::LE; return token::WHERE_OP; }
"!="            { yylval->where_op = usql::WhereStatement::WhereStatementOperator::NE; return token::WHERE_OP; }
">"             { yylval->where_op = usql::WhereStatement::WhereStatementOperator::GT; return token::WHERE_OP; }
"<"             { yylval->where_op = usql::WhereStatement::WhereStatementOperator::LT; return token::WHERE_OP; }
"="             { yylval->where_op = usql::WhereStatement::WhereStatementOperator::EQ; return token::WHERE_OP; }

[0-9]+          {
                    yylval->intdata = atoi(yytext);
                    return token::INTEGER;
                }
[a-zA-Z0-9]+    {
                    yylval->stringdata = new std::string(yytext);
                    return token::IDENTIFIER;
                }
\"(\\.|[^"])*\" {   
                    std::string tmp(yytext);
                    yylval->stringdata = new std::string();
                    *(yylval->stringdata) = tmp.substr(1, tmp.length() - 2);
                    return token::LITERAL_STRING;
                }

.               { return int(yytext[0]); }

%%

