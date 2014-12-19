/* 
* @Author: BlahGeek
* @Date:   2014-12-14
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-14
*/

#ifndef __usql_root_table_h__
#define __usql_root_table_h__ value

#include "./table.h"
#include "./sql_statement.h"
#include "./index.h"

using namespace usql;

namespace usql {

// TODO: Should not inherite from Table?

class RootTable : public Table {

protected:
    static std::vector<column_def_t> getColumnDefs();
    static std::map<std::string, column_constraints_t> getColumnCons();

    const static size_t MAX_DESC_LEN = 1024;

public:

    enum class RowType {
        TABLE, INDEX 
    };

    RootTable(PageIO & page_io, DBMeta & meta);

    std::vector<std::unique_ptr<Table>> getTables();

    std::unique_ptr<Table> getTable(std::string tb_name);
    std::map<std::string, std::unique_ptr<IndexBase>> getIndexes(std::string tb_name);


};

}

#endif
