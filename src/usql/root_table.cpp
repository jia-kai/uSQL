/* 
* @Author: BlahGeek
* @Date:   2014-12-14
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-18
*/

#include <iostream>
#include "./root_table.h"
#include "./datatype/int.h"
#include "./datatype/string.h"

using namespace usql;


std::vector<column_def_t> RootTable::getColumnDefs() {
    std::vector<column_def_t> ret;
    // TABLE or INDEX
    ret.emplace_back("type", std::shared_ptr<DataTypeBase>(new IntDataType()));
    // Name: for table, it's table name
    //       for index, it's the table name + column name
    ret.emplace_back("name", std::shared_ptr<DataTypeBase>(new StringDataType()));
    // Desc: for table, it's a create statement
    //       for index, it's the table name
    ret.emplace_back("desc", std::shared_ptr<DataTypeBase>(new StringDataType(MAX_DESC_LEN)));
    // Root page id
    ret.emplace_back("page_root", std::shared_ptr<DataTypeBase>(new IntDataType()));
    // Row number
    ret.emplace_back("rows", std::shared_ptr<DataTypeBase>(new IntDataType()));
}

std::map<std::string, column_constraints_t> RootTable::getColumnCons() {
    std::map<std::string, column_constraints_t> ret;
    ret[std::string("name")].insert(SQLStatement::ColumnConstraint::PRIMARY);
    return ret;
}

RootTable::RootTable(PageIO & page_io, DBMeta & meta):
    Table(page_io, getColumnDefs(),
          meta.root_table_rows, 
          [&](const rowid_t row){meta.root_table_rows = row; }) {
    this->load(meta.root_table_page, 
               [&](const PageIO::Page & root){meta.root_table_page = root.id(); });
}
