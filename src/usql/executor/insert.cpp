/*
* @Author: BlahGeek
* @Date:   2014-12-18
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
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
            throw SQLException("NOT NULL column not provided");
    }

    if(target_columns.size() != vals.size()) {
        usql_log("Size: %zd, %zd", target_columns.size(), vals.size());
        throw SQLException("Invalid size of values or column names");
    }

    std::vector<LiteralData> full_vals;
    for(size_t i = 0 ; i < target_index.size() ; i += 1) {
        if(target_index[i] == -1)
            full_vals.push_back(tableinfo->table->columns[i].second->make_default());
        else
            full_vals.push_back(vals[target_index[i]]);

        if(!this->check_constraint(tableinfo, i, full_vals.back()))
            throw NotUniqueException("Column not unique");
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
