/* 
* @Author: BlahGeek
* @Date:   2014-12-18
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-19
*/

#include "./insert.h"
#include <algorithm>

using namespace usql;

rowid_t InsertExecutor::insert(const std::vector<LiteralData> & vals) {
    usql_assert(tableinfos.size() == 1, "only one table is needed by insert executor");
    auto & tableinfo = tableinfos.back();
    auto & target_index = target_columns_index.back();

    for(size_t i = 0 ; i < target_index.size() ; i += 1) {
        if(target_index[i] != -1) continue;
        // check not null
        auto & cons = tableinfo->constraints[i];
        if(cons.find(SQLStatement::ColumnConstraint::NOT_NULL) != cons.end())
            throw InsertException("NOT NULL column not provided");
    }

    if(target_columns.size() != vals.size()) {
        usql_log("Size: %lu, %lu", target_columns.size(), vals.size());
        throw InsertException("Invalid size of values or column names");
    }

    std::vector<LiteralData> full_vals;
    for(size_t i = 0 ; i < target_index.size() ; i += 1) {
        if(target_index[i] == -1)
            full_vals.push_back(tableinfo->table->columns[i].second->make_default());
        else 
            full_vals.push_back(vals[target_index[i]]);

        // check primary / unique
        auto & cons = tableinfo->constraints[i];
        auto & val = full_vals.back();
        auto & index = tableinfo->indexes[i];
        if(cons.find(SQLStatement::ColumnConstraint::PRIMARY) != cons.end() || 
           cons.find(SQLStatement::ColumnConstraint::UNIQUE) != cons.end()) {
            usql_assert(index, "Index must exists for Primary or Unique column");
            auto rows = index->find(IndexBase::BoundType::INCLUDE, val,
                                    IndexBase::BoundType::INCLUDE, val);
            if(!rows.empty())
                throw InsertException("Column not unique");
        }
    }

    auto ret = tableinfo->table->insert(full_vals);
    for(size_t i = 0 ; i < target_index.size() ; i += 1) {
        auto & val = full_vals[i];
        auto & index = tableinfo->indexes[i];
        if(index)
            index->insert(val, ret);
    }
    return ret;

}
