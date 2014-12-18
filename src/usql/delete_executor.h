/* 
* @Author: BlahGeek
* @Date:   2014-12-18
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-18
*/

#ifndef __usql_delete_executor_h__
#define __usql_delete_executor_h__ value

#include "./sql_statement.h"
#include "./table.h"
#include "./index.h"
#include "./where_statement.h"
#include "./table_info.h"

using namespace usql;

namespace usql {

class DeleteExecutor {
private:
    std::shared_ptr<TableInfo> tableinfo;

public:
    DeleteExecutor(std::shared_ptr<TableInfo> tableinfo):
        tableinfo(tableinfo) {}
    std::set<rowid_t> execute(std::unique_ptr<WhereStatement> where_stmt);
};

}

#endif
