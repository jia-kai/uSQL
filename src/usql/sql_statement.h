#ifndef __usql_sql_statement_h__
#define __usql_sql_statement_h__ value

#include "./common.h"
#include "./datatype/base.h"

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
    };
    enum class ColumnConstraint {
        PRIMARY, NOT_NULL, UNIQUE
    };

    Type type;
    std::string identifier;

    // TODO: SELECT values, e.g. SUM(xx)

    std::vector<column_def_t> columns;
    std::map<std::string, std::set<ColumnConstraint>> column_constraints;

    std::vector<LiteralData> values;
    // TODO: WHERE statement

public:
    SQLStatement(std::string sql);
    void setDebug(bool enable);
    int parse();
    std::ostream & print(std::ostream & stream);
};

using column_constraints_t = std::set<SQLStatement::ColumnConstraint>;

}

#endif
