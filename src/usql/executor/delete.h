/*
* @Author: BlahGeek
* @Date:   2014-12-18
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#ifndef __usql_delete_executor_h__
#define __usql_delete_executor_h__ value

#include "../parser/sql_statement.h"
#include "../db/table.h"
#include "../db/index.h"
#include "../parser/where_statement.h"
#include "../db/table_info.h"

#include "./base.h"

using namespace usql;

namespace usql {

class DeleteExecutor: public BaseExecutor {

public:
    using BaseExecutor::BaseExecutor;

    std::set<rowid_t> execute(const std::unique_ptr<WhereStatement> & where_stmt) {

        usql_assert(tableinfos.size() == 1, "Only one table is allowed in delete statement");

        std::set<rowid_t> ret;
        this->find(where_stmt, [&](rowid_t rowid, const std::vector<LiteralData> & vals) -> bool {
            ret.insert(rowid);
            auto & tableinfo = tableinfos.back();
            tableinfo->table->erase(rowid);
            for(size_t i = 0 ; i < tableinfo->indexes.size() ; i += 1) {
                auto & index = tableinfo->indexes[i];
                if(index == nullptr) continue;
                index->erase(vals[i], rowid);
            }
            return true;
        }, true); // callback all last table

        usql_log("%zd rows deleted", ret.size());
        return ret;
    }

};

}

#endif
