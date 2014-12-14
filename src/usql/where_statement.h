#ifndef __usql_where_statement_h__
#define __usql_where_statement_h__ value

#include "./common.h"
#include "./datatype/base.h"
#include <map>
#include <utility>
#include <iostream>

using namespace usql;

namespace usql {

using ColumnAndTableName = std::pair<std::string, std::string>;

class WhereStatement {

public:
    enum class WhereStatementType {
        AND, OR, NOT, // for non-leaf node
        LEAF,
        PASS
    };

    WhereStatementType type = WhereStatementType::LEAF;
    std::vector<std::unique_ptr<WhereStatement>> children;

    enum class WhereStatementOperator {
        EQ, NE, GT, LT, GE, LE
    };

    WhereStatementOperator op = WhereStatementOperator::EQ;

    LiteralData a, b;
    ColumnAndTableName na, nb;
    bool a_is_literal = true, b_is_literal = true;

    bool need_verify = false;

public:
    bool verify(std::map<ColumnAndTableName, LiteralData> & data,
                bool force_verify = false);

    static const rowid_t INCLUDE_ALL;

    using table_rows_map_t = std::map<std::string, std::set<rowid_t>>;
    using index_map_t = std::map<ColumnAndTableName, std::unique_ptr<IndexBase>>;

    std::ostream & print(std::ostream & stream) const;

    table_rows_map_t filter(table_rows_map_t & rows,
                            index_map_t & indexes);

    void normalize();
    void setDefaultTable(const std::string & tb_name);

private:
    void revert();
    bool verify_leaf() const;
    table_rows_map_t empty_map(table_rows_map_t rows) const;

    void normalize_leaf();

};

}

#endif
