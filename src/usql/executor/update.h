/* 
* @Author: BlahGeek
* @Date:   2014-12-19
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/
#ifndef __usql_executor_update_h__
#define __usql_executor_update_h__ value

#include "../parser/sql_statement.h"
#include "../db/table.h"
#include "../datatype/base.h"
#include "../parser/where_statement.h"
#include "../db/table_info.h"

#include "./base.h"

using namespace usql;

namespace usql {

class UpdateExecutor: public BaseExecutor {

public:

    using BaseExecutor::BaseExecutor;
    std::set<rowid_t> execute(std::vector<LiteralData> vals,
                              const std::unique_ptr<WhereStatement> & where_stmt) {
        usql_assert(tableinfos.size() == 1, "only one table is needed by update executor");
        auto & tableinfo = tableinfos.back();
        auto & target_index = target_columns_index.back();

        std::set<rowid_t> ret;

        this->find(where_stmt, [&](rowid_t rowid, const std::vector<LiteralData> & orig) -> bool {
            std::vector<LiteralData> update_val;
            for(size_t i = 0 ; i < target_index.size() ; i += 1) {
                if(target_index[i] == -1)
                    update_val.push_back(orig[i]);
                else {
                    update_val.push_back(vals[target_index[i]]);
                    if(!this->check_constraint(tableinfo, i, update_val.back()))
                        throw "Not unique";
                }
            }

            #if 0
            usql_log("Update values:");
            for(auto & val: update_val) {
                val.print(std::cerr);
                std::cerr << std::endl;
            }
            #endif

            auto new_rowid = tableinfo->table->insert(update_val, rowid);
            usql_assert(new_rowid == rowid, "new rowid after update is not same");

            for(size_t i = 0 ; i < target_index.size() ; i += 1) {
                if(target_index[i] == -1) continue; // not changed
                auto & index = tableinfo->indexes[i];
                if(index == nullptr) continue; // not indexed
                index->erase(orig[i], rowid);
                index->insert(update_val[i], rowid);
            }
            ret.insert(rowid);
            return false;
        }, true); // callback all last table
        return ret;
    }

};

}

#endif
