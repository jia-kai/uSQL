#ifndef __usql_sql_statement_h__
#define __usql_sql_statement_h__ value

#include "./common.h"
#include "./datatype/base.h"

#include <vector>
#include <map>
#include <set>

using namespace usql;

namespace usql {

class SQLParser;
class SQLScanner;

class SQLStatement {

private:
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

    std::vector<std::pair<std::string, std::unique_ptr<DataTypeBase>>> columns;
    std::map<std::string, std::set<ColumnConstraint>> column_constraints;

    std::vector<DataBase> values;
    // TODO: WHERE statement

public:
    SQLStatement(std::string sql);
    std::ostream & print(std::ostream & stream);
};

}

#endif
