#ifndef __usql_select_excutor_h__
#define __usql_select_excutor_h__ value

#include "../parser/sql_statement.h"
#include "../db/table.h"
#include "../datatype/base.h"
#include "../parser/where_statement.h"
#include "../db/table_info.h"

#include "./base.h"

using namespace usql;

namespace usql {

class SelectExecutor: public BaseExecutor {

private:
    auto expand_dests(std::vector<ColumnAndTableName> dests) -> decltype(dests);

public:
    SelectExecutor(std::vector<std::shared_ptr<TableInfo>> tableinfos, 
                   std::vector<ColumnAndTableName> target_columns):
    BaseExecutor(tableinfos) {
        this->setTargetColumns(expand_dests(target_columns));
    }

    // using find = BaseExecutor::find;
};

}

#endif
