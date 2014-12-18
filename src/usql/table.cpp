/* 
* @Author: BlahGeek
* @Date:   2014-12-04
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-18
*/

#include <iostream>
#include "./table.h"

using namespace usql;

size_t Table::compute_payload_size(std::vector<column_def_t> & cols) {
    size_t ret = 0;
    for(auto & col: cols) {
        ret += col.second->storage_size();
    }
    return ret;
}

Table::Table(PageIO & page_io,
             std::vector<column_def_t> cols, 
             std::map<std::string, column_constraints_t> cons,
             rowid_t mr,
             rowid_updator_t updator):
BTree<rowid_t>(page_io, compute_payload_size(cols)),
columns(cols),
maxrow(mr),
maxrow_updator(updator){

    for(auto & col: columns)
        constraints.push_back(cons[col.first]);

}

rowid_t Table::insert(std::vector<LiteralData> values, rowid_t target) {
    if(target == -1) {
        target = maxrow;
        maxrow += 1;
        if(maxrow_updator != nullptr)
            maxrow_updator(maxrow);
    }
    auto it = this->lookup(target, true);
    char * pdata = static_cast<char *>(it.payload());

    for(size_t i = 0 ; i < columns.size() ; i += 1) {
        columns[i].second->dump(pdata, values[i]);
        pdata += columns[i].second->storage_size();
    }

    return it.key();
}

std::vector<LiteralData> Table::load_data(Table::Iterator & it) {
    usql_assert(it.valid(), "iterator invalid");
    std::vector<LiteralData> ret;

    char * pdata = static_cast<char *>(it.payload());
    for(size_t i = 0 ; i < columns.size() ; i += 1) {
        ret.push_back(columns[i].second->load(pdata));
        pdata += columns[i].second->storage_size();
    }
    return std::move(ret);
}

std::vector<LiteralData> Table::find(rowid_t rowid) {
    auto it = this->lookup(rowid, false);
    if(!it.valid() || it.key() != rowid) 
        return std::vector<LiteralData>();

    return load_data(it);
}

void Table::walkthrough(row_callback_t callback) {
    auto it = this->lookup(0, false);
    while(it.valid()) {
        auto continue_ = callback(*this, load_data(it));
        if(!continue_)
            break;
        it.next();
    }
}
