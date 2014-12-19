/* 
* @Author: BlahGeek
* @Date:   2014-12-19
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-19
*/

#include <iostream>
#include "./base.h"

using namespace usql;

void BaseExecutor::setTargetColumns(std::vector<ColumnAndTableName> target_columns) {
    this->target_columns = target_columns;

    for(auto & tableinfo: tableinfos) {
        target_columns_index.emplace_back();
        for(auto & col: tableinfo->table->columns) {
            auto it = std::find_if(target_columns.begin(), target_columns.end(),
                                   [&](const ColumnAndTableName & x) -> bool {
                                     return x.first == tableinfo->name 
                                        && x.second == col.first;
                                   });
            if(it == target_columns.end()) target_columns_index.back().push_back(-1);
            else target_columns_index.back().push_back(it - target_columns.begin());
        }
        usql_log("Target columns index for table %s:", tableinfo->name.c_str());
        for(size_t i = 0 ; i < target_columns_index.back().size() ; i += 1)
            usql_log("\t%lu -> %d", i, target_columns_index.back()[i]);
    }
}

void BaseExecutor::setFullColumns() {
    std::vector<ColumnAndTableName> target;
    for(auto & tableinfo: tableinfos) {
        for(auto & col: tableinfo->table->columns)
            target.emplace_back(tableinfo->name, col.first);
    }
    this->setTargetColumns(target);
}

void BaseExecutor::find(const std::unique_ptr<WhereStatement> & where,
                        BaseExecutor::callback_t callback) {
    std::vector<std::set<rowid_t>> rows(tableinfos.size());
    for(auto & row: rows)
        row.insert(WhereStatement::INCLUDE_ALL);
    where->prepare(tableinfos);
    rows = where->filter(rows);
    for(size_t i = 0 ; i < rows.size() ; i += 1) {
        usql_log("After filter: row count of `%s`: %lu", 
                 tableinfos[i]->name.c_str(), rows[i].size());
        if(rows[i].find(WhereStatement::INCLUDE_ALL) != rows[i].end())
            usql_log("\tINCLUDE ALL");
    }
    callback_values.resize(target_columns.size());
    verify_values.resize(tableinfos.size());
    this->recursive_find(0, rows, where, callback);
}


void BaseExecutor::recursive_find(size_t depth, 
                                  std::vector<std::set<rowid_t>> & rows,
                                  const std::unique_ptr<WhereStatement> & where,
                                  BaseExecutor::callback_t callback) {
    auto table_name = tableinfos[depth]->name;
    auto & table = tableinfos[depth]->table;

    auto callnext_f = [&, this](rowid_t rowid, const std::vector<LiteralData> & data) -> bool {
        auto & dests_index = target_columns_index[depth];
        verify_values[depth].clear();
        for(size_t i = 0 ; i < data.size() ; i += 1) {
            verify_values[depth].push_back(data[i]);
            if(dests_index[i] != -1)
                callback_values[dests_index[i]] = data[i];
        }
        if(depth + 1 == tableinfos.size()) {
            if(where->verify(verify_values))
                return callback(rowid, callback_values); // rowid means the rowid of the last table
            else return false;
        } else {
            recursive_find(depth+1, rows, where, callback);
            return false;
        }
    };

    auto & this_rows = rows[depth];
    if(this_rows.find(WhereStatement::INCLUDE_ALL) != this_rows.end()) {
        table->walkthrough(callnext_f);
    } else {
        for(auto rowid: this_rows) {
            auto data = table->find(rowid);
            callnext_f(rowid, data);
        }
    }
}
