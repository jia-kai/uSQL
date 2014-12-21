/* 
* @Author: BlahGeek
* @Date:   2014-12-19
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/
#ifndef __usql_executor_base_h__
#define __usql_executor_base_h__ value

#include "../parser/sql_statement.h"
#include "../db/table.h"
#include "../db/index.h"
#include "../parser/where_statement.h"
#include "../db/table_info.h"

using namespace usql;

namespace usql {

class BaseExecutor {
protected:
    using callback_t = row_callback_t;

private:
    std::vector<LiteralData> callback_values;
    std::vector<std::vector<LiteralData>> verify_values;

    void recursive_find(size_t depth, 
                        std::vector<std::set<rowid_t>> & rows,
                        const std::unique_ptr<WhereStatement> & where,
                        callback_t callback,
                        bool callback_all);

protected:
    std::vector<std::shared_ptr<TableInfo>> tableinfos;
    std::vector<ColumnAndTableName> target_columns;

    std::vector<std::vector<int>> target_columns_index; // table.col -> target_columns

protected:
    void setTargetColumns(std::vector<ColumnAndTableName> target_columns);
    void setFullColumns();

    bool check_constraint(std::shared_ptr<TableInfo> tableinfo, 
                          size_t number, const LiteralData & val);

public:
    BaseExecutor(std::vector<std::shared_ptr<TableInfo>> tableinfos):
        tableinfos(tableinfos) {
        }
    BaseExecutor(std::vector<std::shared_ptr<TableInfo>> tableinfos, 
                 std::vector<ColumnAndTableName> target_columns):
        tableinfos(tableinfos) {
            this->setTargetColumns(target_columns);
        }
    void find(const std::unique_ptr<WhereStatement> & where, 
              callback_t callback, bool callback_all = false);
};

}

#endif
