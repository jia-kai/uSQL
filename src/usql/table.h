#ifndef __usql_table_h__
#define __usql_table_h__ value

#include "./parser/sql.tab.hpp"
#include "./parser/sql_scanner.h"
#include "./sql_statement.h"
#include "./ds/btree.h"

using namespace usql;

namespace usql {

using rowid_t = int64_t;
using rowid_updator_t = std::function<void(const rowid_t)>;

class Table;

using row_callback_t = std::function<bool(const Table &, const std::vector<LiteralData> &)>;

class Table: public BTree<rowid_t> {
private:
    static size_t compute_payload_size(std::vector<column_def_t> & cols);

public:
    std::vector<column_def_t> columns;
    std::map<std::string, column_constraints_t> constraints;

protected:
    rowid_t maxrow = 0;
    rowid_updator_t maxrow_updator = nullptr;

    std::vector<LiteralData> load_data(Iterator & it);

public:

    Table(PageIO & page_io,
        std::vector<column_def_t> cols, 
        std::map<std::string, column_constraints_t> cons,
        rowid_t mr,
        rowid_updator_t updator);

    // insert: target = -1 to insert new record at maxrow
    rowid_t insert(std::vector<LiteralData> values, rowid_t target = -1);
    std::vector<LiteralData> find(rowid_t rowid); // return empty vector if not found
    // erase(row_id_t) is inherited
    void walkthrough(row_callback_t callback);
};

}

#endif
