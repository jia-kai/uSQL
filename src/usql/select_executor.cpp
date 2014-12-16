/* 
* @Author: BlahGeek
* @Date:   2014-12-06
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-16
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
            for(auto & table: tables) {
                expanded_table.emplace_back(table.first, dest.second);
            }
        } else 
            expanded_table.push_back(dest);
    }
    std::vector<ColumnAndTableName> ret;
    for(auto & dest: expanded_table) {
        if(dest.second == "*") {
            auto table = std::find_if(tables.begin(), tables.end(), 
                                      [&](const std::pair<std::string, std::shared_ptr<Table>> & x) -> bool {
                                          return x.first == dest.first;
                                      });
            usql_assert(table != tables.end(), 
                        "table %s for found for expand", dest.first.c_str());
            for(auto & col: table->second->columns)
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

    std::map<std::string, std::set<rowid_t>> rows;
    for(auto & table: tables)
        rows[table.first].insert(WhereStatement::INCLUDE_ALL);
    rows = where->filter(rows, indexes);

    for(auto & row: rows) {
        usql_log("After filter: row count of `%s`: %lu", 
                 row.first.c_str(), row.second.size());
        if(row.second.find(WhereStatement::INCLUDE_ALL) != row.second.end())
            usql_log("\tINCLUDE ALL");
    }

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

        table_columns_count.push_back(table->columns.size());

        usql_log("Dests_index for table %lu:", depth);
        for(size_t i = 0 ; i < dests_index.size() ; i += 1)
            usql_log("\t%lu -> %d", i, dests_index[i]);
    }

    auto callnext_f = [&, this](const std::vector<LiteralData> & data) -> bool {
        // WTF: must calculate dests_index every time we use it
        // because the memory location of dests_indexes may change
        auto & dests_index = dests_indexes[depth];
        auto verify_val_index_base = (depth == 0)?0:
            table_columns_count[depth-1];
        for(size_t i = 0 ; i < data.size() ; i += 1) {
            verify_values[verify_val_index_base + i] = data[i];
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
