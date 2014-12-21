/* 
* @Author: BlahGeek
* @Date:   2014-12-20
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/
#ifndef __usql_sql_runner_h__
#define __usql_sql_runner_h__  

#include "./executor/base.h"
#include "./executor/delete.h"
#include "./executor/insert.h"
#include "./executor/select.h"
#include "./executor/update.h"
#include "./sql_statement.h"
#include "./dbmeta.h"
#include "./table_info.h"

using namespace usql;

namespace usql {

class SQLRunner {
public:
    using callback_t = std::function<void(const std::vector<LiteralData> & vals)>;

private:
    PageIO & page_io;
    std::shared_ptr<TableInfo> root_table;

    void _update(std::string name, std::string col, LiteralData val);
    void findTableIndexes(std::shared_ptr<TableInfo> & tableinfo);
    std::shared_ptr<TableInfo> getTableInfo(std::string tbname);

protected:
    void createTable(std::string tbname, 
                     const std::vector<column_def_t> & cols,
                     std::string sql);
    void createIndex(const std::shared_ptr<Table> & table,
                     ColumnAndTableName name,
                     int col_index);

public:
    SQLRunner(PageIO & page_io);
    void run(const std::unique_ptr<SQLStatement> & stmt,
             callback_t callback);
};

}

#endif
