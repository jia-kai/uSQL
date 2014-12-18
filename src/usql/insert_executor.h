/* 
* @Author: BlahGeek
* @Date:   2014-12-18
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-18
*/

#ifndef __usql_insert_executor_h__
#define __usql_insert_executor_h__ value

#include "./sql_statement.h"
#include "./table.h"
#include "./index.h"

using namespace usql;

namespace usql {

class InsertException: public std::exception {
private:
    std::string reason;
public:
    InsertException(std::string s): reason(s) {}
    const char * what() const noexcept override {return reason.c_str(); }
};

class InsertExecutor {
private:
    std::shared_ptr<Table> table;
    std::vector<std::string> column_names;

    std::vector<std::shared_ptr<IndexBase>> indexes;
    std::vector<int> column_indexes;

public:
    InsertExecutor(std::shared_ptr<Table> t, 
                   std::vector<ColumnAndTableName> cols);
    void addIndex(std::string name, std::shared_ptr<IndexBase> index);

public:
    rowid_t insert(const std::vector<LiteralData> & vals);
};

}

#endif
