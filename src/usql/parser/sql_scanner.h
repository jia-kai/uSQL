#ifndef __usql__table_schema_scanner_h__
#define __usql__table_schema_scanner_h__ value

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#undef  YY_DECL
#define YY_DECL int usql::SQLScanner::yylex()

#include "./sql.tab.hpp"

using namespace usql;

namespace usql {

class SQLScanner: public yyFlexLexer {
public:
    SQLScanner(std::istream * in): yyFlexLexer(in) {}
    void setDebug(bool enable) {yy_flex_debug = enable; }
    int yylex(usql::SQLParser::semantic_type *lval) {
        yylval = lval;
        return( yylex() ); 
    }

private:
    int yylex();
    usql::SQLParser::semantic_type *yylval = nullptr;

};

}

#endif
