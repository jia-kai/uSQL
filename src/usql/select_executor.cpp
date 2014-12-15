/* 
* @Author: BlahGeek
* @Date:   2014-12-06
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-15
*/

#include <iostream>
#include <algorithm>
#include "./select_executor.h"
#include "./where_statement.h"
#include "./index.h"

using namespace usql;


void SelectExecutor::execute(std::vector<ColumnAndTableName> dests, 
                             const std::unique_ptr<WhereStatement> & where,
                             SelectExecutor::callback_t callback) {
    std::map<std::string, std::set<rowid_t>> rows;
    for(auto & table: tables)
        rows[table.first].insert(WhereStatement::INCLUDE_ALL);
    where->filter(rows, indexes);

    std::vector<ColumnAndTableName> names;
    for(auto & table: tables) {
        auto tb_name = table.first;
        for(auto & col: table.second->columns)
            names.emplace_back(tb_name, col.first);
    }
    where->prepare_verify(names);

    dests_indexes.clear();
    callback_values.resize(dests.size());
    verify_values.resize(names.size());
    this->recursive_execute(0, rows, dests, where, callback);
}

void SelectExecutor::recursive_execute(size_t depth, 
                                       std::map<std::string, std::set<rowid_t>> & rows,
                                       const std::vector<ColumnAndTableName> dests,
                                       const std::unique_ptr<WhereStatement> & where,
                                       SelectExecutor::callback_t callback) {
    if(depth >= tables.size()) {
        if(where->verify(verify_values))
            callback(callback_values);
        return;
    }
    auto table_name = tables[depth].first;
    auto & table = tables[depth].second;
    // calculate which columns is in dests
    if(dests_indexes.size() <= depth) {
        std::vector<int> dests_index;
        for(size_t i = 0 ; i < table->columns.size() ; i += 1) {
            auto & col = table->columns[i];
            auto it = std::find(dests.begin(), dests.end(), 
                                ColumnAndTableName(table_name, col.first));
            if(it != dests.end())
                dests_index.push_back(it - dests.begin());
            else dests_index.push_back(-1);
        }
        dests_indexes.push_back(dests_index);
    }
    auto & dests_index = dests_indexes[depth];

    auto callnext_f = [&, this](const std::vector<LiteralData> & data) -> bool {
        for(size_t i = 0 ; i < data.size() ; i += 1) {
            verify_values[i] = data[i];
            if(dests_index[i] != -1)
                callback_values[dests_index[i]] = data[i];
        }
        recursive_execute(depth+1, rows, dests, where, callback);
        return true;
    };

    auto & this_rows = rows[table_name];
    if(this_rows.find(WhereStatement::INCLUDE_ALL) != this_rows.end()) {
        table->walkthrough([&, this](const Table & _table, const std::vector<LiteralData> & data) -> bool {
            return callnext_f(data);
        });
    } else {
        for(auto rowid: this_rows) {
            auto data = table->find(rowid);
            callnext_f(data);
        }
    }
}
