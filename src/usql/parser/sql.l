%{
#include <iostream>
#include <string>
#include "sql_scanner.h"
#include "../datatype/base.h"
#include "./sql_statement.h"

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

[ \t\r\n]+          { }

(?i:"INT")          { return token::INT; }
(?i:"VARCHAR")      { return token::VARCHAR; }

(?i:"PRIMARY KEY")  {
                        yylval->column_constraint = usql::SQLStatement::ColumnConstraint::PRIMARY;
                        return token::COLUMN_CONSTRAINT;
                    }
(?i:"NOT NULL")     {
                        yylval->column_constraint = usql::SQLStatement::ColumnConstraint::NOT_NULL;
                        return token::COLUMN_CONSTRAINT;
                    }
(?i:"UNIQUE")       {
                        yylval->column_constraint = usql::SQLStatement::ColumnConstraint::UNIQUE;
                        return token::COLUMN_CONSTRAINT;
                    }

(?i:"CREATE")       { return token::CREATE; }
(?i:"TABLE")        { return token::TABLE; }
(?i:"TABLES")       { return token::TABLES; }

(?i:"SELECT")       { return token::SELECT; }
(?i:"FROM")         { return token::FROM; }
(?i:"WHERE")        { return token::WHERE; }
(?i:"INSERT")       { return token::INSERT; }
(?i:"INTO")         { return token::INTO; }
(?i:"VALUES")       { return token::VALUES; }
(?i:"DELETE")       { return token::DELETE; }
(?i:"UPDATE")       { return token::UPDATE; }
(?i:"SET")          { return token::SET; }
(?i:"SHOW")         { return token::SHOW; }
(?i:"INDEX")        { return token::INDEX; }
(?i:"ON")           { return token::ON; }
(?i:"DROP")         { return token::DROP; }
(?i:"USE")          { return token::USE; }
(?i:"DATABASE")     { return token::DATABASE; }

(?i:"AND")          { return token::AND; }
(?i:"OR")           { return token::OR; }
(?i:"NOT")          { return token::NOT; }

(?i:"NULL")         { return token::NULL_; }

">="                { yylval->where_op = usql::WhereStatement::WhereStatementOperator::GE; return token::WHERE_OP; }
"<="                { yylval->where_op = usql::WhereStatement::WhereStatementOperator::LE; return token::WHERE_OP; }
"!="                { yylval->where_op = usql::WhereStatement::WhereStatementOperator::NE; return token::WHERE_OP; }
">"                 { yylval->where_op = usql::WhereStatement::WhereStatementOperator::GT; return token::WHERE_OP; }
"<"                 { yylval->where_op = usql::WhereStatement::WhereStatementOperator::LT; return token::WHERE_OP; }
"="                 { yylval->where_op = usql::WhereStatement::WhereStatementOperator::EQ; return token::WHERE_OP; }
(?i:"is")           { yylval->where_op = usql::WhereStatement::WhereStatementOperator::EQ; return token::WHERE_OP; }

-?[0-9]+            {
                        yylval->intdata = atoi(yytext);
                        return token::INTEGER;
                    }
[a-zA-Z0-9_]+        {
                        yylval->stringdata = new std::string(yytext);
                        return token::IDENTIFIER;
                    }
\"(\\.|[^"])*\"     {   
                        std::string tmp(yytext);
                        yylval->stringdata = new std::string();
                        // FIXME
                        *(yylval->stringdata) = tmp.substr(1, tmp.length() - 2);
                        return token::LITERAL_STRING;
                    }
\'(\\.|[^'])*\'     {   
                        std::string tmp(yytext);
                        yylval->stringdata = new std::string();
                        // FIXME
                        *(yylval->stringdata) = tmp.substr(1, tmp.length() - 2);
                        return token::LITERAL_STRING;
                    }

.                   { return int(yytext[0]); }

%%

