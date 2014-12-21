/* 
* @Author: BlahGeek
* @Date:   2014-12-18
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#ifndef __usql_table_info_h__
#define __usql_table_info_h__ value

#include "./table.h"
#include "./index.h"
#include "./sql_statement.h"
#include "./page_io.h"

using namespace usql;

namespace usql {

class TableInfo {
public:
    std::string name;
    
    std::shared_ptr<Table> table;
    // size = column count, nullptr if not present
    std::vector<std::shared_ptr<IndexBase>> indexes;
    std::vector<column_constraints_t> constraints;

    void setConstraints(std::map<std::string, column_constraints_t> x) {
        constraints.clear();
        for(auto & col: table->columns)
            constraints.push_back(x[col.first]);
    }

public:
    enum class RootRowType {
        TABLE, INDEX 
    };

    static std::shared_ptr<TableInfo> getRootTableInfo(PageIO & page_io);
};

}

#endif
