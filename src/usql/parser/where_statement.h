#ifndef __usql_where_statement_h__
#define __usql_where_statement_h__ value

#include "../common.h"
#include "../datatype/base.h"
#include <map>
#include <utility>
#include <iostream>

using namespace usql;

namespace usql {

class TableInfo;

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
    void normalize();
    void setDefaultTable(const std::string & tb_name);
    std::ostream & print(std::ostream & stream) const;

private:
    int na_table_i = -1, na_col_i = -1;
    int nb_table_i = -1, nb_col_i = -1;
    // int verify_index_a = -1, verify_index_b = -1; // index in verify_data
    // int table_index_a = -1, table_index_a = -1; // index in rows
    // int index_index_a = -1, index_index_a = -1; // index in indexes

    std::vector<std::shared_ptr<TableInfo>> tableinfos;

public:
    void prepare(const std::vector<std::shared_ptr<TableInfo>> & tableinfos);

    bool verify(const std::vector<std::vector<LiteralData>> & data,
                bool force_verify = false);

    static const rowid_t INCLUDE_ALL;

    using table_rows_map_t = std::vector<std::set<rowid_t>>;

    table_rows_map_t filter(table_rows_map_t & rows);

private:
    void revert();
    bool verify_leaf() const;
    table_rows_map_t empty_map(table_rows_map_t rows) const;

    void normalize_leaf();

};

}

#endif
