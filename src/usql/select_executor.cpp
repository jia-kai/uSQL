/* 
* @Author: BlahGeek
* @Date:   2014-12-06
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-19
*/

#include <iostream>
#include <algorithm>
#include "./select_executor.h"
#include "./where_statement.h"
#include "./index.h"

using namespace usql;

auto SelectExecutor::expand_dests(std::vector<ColumnAndTableName> dests) -> decltype(dests) {
    std::vector<ColumnAndTableName> expanded_table;
    for(auto & dest: dests) {
        if(dest.first == "*") {
            for(auto & tableinfo: tableinfos) {
                expanded_table.emplace_back(tableinfo->name, dest.second);
            }
        } else 
            expanded_table.push_back(dest);
    }
    std::vector<ColumnAndTableName> ret;
    for(auto & dest: expanded_table) {
        if(dest.second == "*") {
            auto tbinfo = std::find_if(tableinfos.begin(), 
                                       tableinfos.end(), 
                                       [&](const std::shared_ptr<TableInfo> & x) -> bool {
                                           return x->name == dest.first;
                                       });
            usql_assert(tbinfo != tableinfos.end(), 
                        "table %s for found for expand", dest.first.c_str());
            for(auto & col: (*tbinfo)->table->columns)
                ret.emplace_back(dest.first, col.first);
        } else 
            ret.push_back(dest);
    }
    return ret;
}

void SelectExecutor::execute(std::vector<ColumnAndTableName> & dests, 
                             const std::unique_ptr<WhereStatement> & where,
                             SelectExecutor::callback_t callback) {
    dests = expand_dests(dests);

    std::vector<std::set<rowid_t>> rows(tableinfos.size());
    for(auto & row: rows)
        row.insert(WhereStatement::INCLUDE_ALL);

    // this is important
    where->prepare(tableinfos);

    rows = where->filter(rows);

    for(size_t i = 0 ; i < rows.size() ; i += 1) {
        usql_log("After filter: row count of `%s`: %lu", 
                 tableinfos[i]->name.c_str(), rows[i].size());
        if(rows[i].find(WhereStatement::INCLUDE_ALL) != rows[i].end())
            usql_log("\tINCLUDE ALL");
    }

    dests_indexes.clear();
    callback_values.resize(dests.size());
    verify_values.resize(tableinfos.size());
    this->recursive_execute(0, rows, dests, where, callback);
}

void SelectExecutor::recursive_execute(size_t depth, 
                                       std::vector<std::set<rowid_t>> & rows,
                                       const std::vector<ColumnAndTableName> dests,
                                       const std::unique_ptr<WhereStatement> & where,
                                       SelectExecutor::callback_t callback) {
    if(depth >= tableinfos.size()) {
    #if 0
        usql_log("Verifying...");
        for(auto & val: verify_values) {
            val.print(std::cerr); 
            std::cerr << "(" << int(val.datatype) << ") ";
        }
        std::cerr << std::endl;
    #endif
        if(where->verify(verify_values))
            callback(callback_values);
        return;
    }
    auto table_name = tableinfos[depth]->name;
    auto & table = tableinfos[depth]->table;
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

        usql_log("Dests_index for table %lu:", depth);
        for(size_t i = 0 ; i < dests_index.size() ; i += 1)
            usql_log("\t%lu -> %d", i, dests_index[i]);
    }

    auto callnext_f = [&, this](const std::vector<LiteralData> & data) -> bool {
        // WTF: must calculate dests_index every time we use it
        // because the memory location of dests_indexes may change
        auto & dests_index = dests_indexes[depth];
        verify_values[depth].clear();
        for(size_t i = 0 ; i < data.size() ; i += 1) {
            verify_values[depth].push_back(data[i]);
            if(dests_index[i] != -1)
                callback_values[dests_index[i]] = data[i];
        }
        recursive_execute(depth+1, rows, dests, where, callback);
        return false;
    };

    auto & this_rows = rows[depth];
    if(this_rows.find(WhereStatement::INCLUDE_ALL) != this_rows.end()) {
        table->walkthrough([&, this](rowid_t rowid, const std::vector<LiteralData> & data) -> bool {
            return callnext_f(data);
        });
    } else {
        for(auto rowid: this_rows) {
            auto data = table->find(rowid);
            callnext_f(data);
        }
    }
}
