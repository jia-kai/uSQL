#ifndef __usql_sql_statement_h__
#define __usql_sql_statement_h__ value

#include "./common.h"
#include "./datatype/base.h"
#include "./where_statement.h"

#include <vector>
#include <strstream>
#include <map>
#include <set>

using namespace usql;

namespace usql {

class SQLParser;
class SQLScanner;

using column_def_t = std::pair<std::string, std::shared_ptr<DataTypeBase>>;

class SQLStatement {

private:
    std::string origin;
    std::unique_ptr<std::istrstream> st = nullptr;

    std::unique_ptr<SQLParser> parser = nullptr;
    std::unique_ptr<SQLScanner> scanner = nullptr;
public:

    enum class Type {
        CREATE_DB, DROP_DB, USE_DB, 
        SHOW_TBS, CREATE_TB, DROP_TB, DESC_TB,
        INSERT, DELETE, UPDATE,
        CREATE_IDX, DROP_IDX,
        SELECT,
    };
    enum class ColumnConstraint {
        PRIMARY, NOT_NULL, UNIQUE
    };

    Type type;

    std::string database_name; // for *_DB

    std::vector<std::string> table_names; // for *_TB, delete, update, *_IDX, select

    // TODO: SELECT values, e.g. SUM(xx)

    std::vector<column_def_t> column_defs; // for create_tb
    std::map<std::string, std::set<ColumnConstraint>> column_constraints; // for create_tb

    std::vector<std::vector<LiteralData>> values; // for insert

    std::map<std::string, LiteralData> update_vals; // for update

    std::unique_ptr<WhereStatement> where_stmt = nullptr; // for delete, update, select

public:
    SQLStatement(std::string sql);
    void setDebug(bool enable);
    int parse();
    std::ostream & print(std::ostream & stream);
};

using column_constraints_t = std::set<SQLStatement::ColumnConstraint>;

}

#endif
