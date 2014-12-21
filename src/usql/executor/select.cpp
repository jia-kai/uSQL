/* 
* @Author: BlahGeek
* @Date:   2014-12-06
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <iostream>
#include <algorithm>
#include "./select.h"
#include "../db/index.h"

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
                        "table %s not found for expand", dest.first.c_str());
            for(auto & col: (*tbinfo)->table->columns)
                ret.emplace_back(dest.first, col.first);
        } else 
            ret.push_back(dest);
    }
    return ret;
}
