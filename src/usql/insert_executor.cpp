/* 
* @Author: BlahGeek
* @Date:   2014-12-18
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-18
*/

#include "./insert_executor.h"
#include <algorithm>

using namespace usql;

rowid_t InsertExecutor::insert(const std::vector<LiteralData> & vals) {
    if(column_indexes.size() != column_names.size()) {
        for(size_t i = 0 ; i < table->columns.size() ; i += 1) {
            auto & col = table->columns[i];
            auto name = col.first;
            auto it = std::find(column_names.begin(), column_names.end(), name);
            if(it != column_names.end())
                column_indexes.push_back(it - column_names.begin());
            else {
                column_indexes.push_back(-1);
                // check not null
                auto & cons = tableinfo->constraints[i];
                if(cons.find(SQLStatement::ColumnConstraint::NOT_NULL) != cons.end())
                    throw InsertException("NOT NULL column not provided");
            }
        }
    }

    if(column_names.size() != vals.size()) {
        usql_log("Size: %lu, %lu", column_names.size(), vals.size());
        throw InsertException("Invalid size of values or column names");
    }

    std::vector<LiteralData> full_vals;
    for(size_t i = 0 ; i < table->columns.size() ; i += 1) {
        if(column_indexes[i] == -1)
            full_vals.push_back(table->columns[i].second->make_default());
        else 
            full_vals.push_back(vals[column_indexes[i]]);

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

    auto ret = table->insert(full_vals);
    for(size_t i = 0 ; i < table->columns.size() ; i += 1) {
        auto & val = full_vals[i];
        auto & index = tableinfo->indexes[i];
        if(index)
            index->insert(val, ret);
    }
    return ret;

}

InsertExecutor::InsertExecutor(std::shared_ptr<TableInfo> tableinfo, 
                               std::vector<ColumnAndTableName> cols) {
    this->tableinfo = tableinfo;
    this->table = tableinfo->table;

    if(cols.empty()) {
        for(auto & col: table->columns)
            column_names.push_back(col.first);
    } else {
        for(auto & col: cols)
            column_names.push_back(col.second);
    }
}
