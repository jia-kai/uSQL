/* 
* @Author: BlahGeek
* @Date:   2014-12-18
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-19
*/

#include <iostream>
#include "./delete.h"

using namespace usql;

std::set<rowid_t> DeleteExecutor::execute(std::unique_ptr<WhereStatement> where_stmt) {
    std::vector<std::set<rowid_t>> rows(1);
    rows[0].insert(WhereStatement::INCLUDE_ALL);

    where_stmt->prepare({tableinfo});
    rows = where_stmt->filter(rows);
    auto filtered = rows[0];

    usql_log("After filter: %lu rows to delete", filtered.size());

    std::set<rowid_t> ret;

    auto erase_f = [&](rowid_t rowid, const std::vector<LiteralData> & vals) -> bool {
        if(!where_stmt->verify({vals}))
            return false;
        ret.insert(rowid);

        tableinfo->table->erase(rowid);
        for(size_t i = 0 ; i < tableinfo->indexes.size() ; i += 1) {
            auto & index = tableinfo->indexes[i];
            if(index == nullptr) continue;
            index->erase(vals[i], rowid);
        }
        return true;
    };

    if(filtered.find(WhereStatement::INCLUDE_ALL) != filtered.end()) 
        tableinfo->table->walkthrough(erase_f);
    else {
        for(auto rowid: filtered) {
            auto vals = tableinfo->table->find(rowid);
            erase_f(rowid, vals);
        }
    }

    usql_log("After verify: %lu rows deleted", ret.size());
    return ret;
}
