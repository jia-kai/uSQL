/* 
* @Author: BlahGeek
* @Date:   2014-12-20
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <iostream>
#include "./table_info.h"
#include "../common.h"
#include "../datatype/int.h"
#include "../datatype/string.h"

using namespace usql;

std::shared_ptr<TableInfo> TableInfo::getRootTableInfo(PageIO & page_io) {
    //
    // Columns Defs
    //
    std::vector<column_def_t> cols;
    // RootRowType: TABLE or INDEX
    cols.emplace_back("type", std::shared_ptr<DataTypeBase>(new IntDataType()));
    // Name: for table, it's table name
    //       for index, it's the table name + column name
    cols.emplace_back("name", std::shared_ptr<DataTypeBase>(new StringDataType()));
    // Desc: for table, it's a create statement
    //       for index, it's the table name
    cols.emplace_back("desc", std::shared_ptr<DataTypeBase>(new StringDataType(1024)));
    // Root page id
    cols.emplace_back("page_root", std::shared_ptr<DataTypeBase>(new IntDataType()));
    // Row number, only for table
    cols.emplace_back("rows", std::shared_ptr<DataTypeBase>(new IntDataType()));
    // Column index, only for index
    cols.emplace_back("col_index", std::shared_ptr<DataTypeBase>(new IntDataType()));
    //
    // Columns Constraints
    // 
    std::map<std::string, column_constraints_t> cons;
    // name must be unique
    cons["name"].insert(SQLStatement::ColumnConstraint::PRIMARY);

    // load table
    const auto & meta = page_io.file_io().get_meta();
    auto table = std::make_shared<Table>(page_io, cols, meta.root_table_rows, 
                                         [&](const rowid_t & rowid) {
                                            page_io.file_io().update_meta_start().root_table_rows = rowid;
                                            page_io.file_io().update_meta_finish();
                                         });
    table->load(meta.root_table_page, [&](const PageIO::Page & p) {
        page_io.file_io().update_meta_start().root_table_page = p.id();
        page_io.file_io().update_meta_finish();
    });

    auto ret = std::make_shared<TableInfo>();
    ret->name = "RootTable";
    ret->table = table;
    // load index for `name`
    for(auto & col: ret->table->columns) {
        if(col.first != "name") ret->indexes.push_back(nullptr);
        else {
            auto index = col.second->load_index(page_io, meta.root_index_page, 
                                                [&](const PageIO::Page & p) {
                                                    page_io.file_io().update_meta_start().root_index_page = p.id();
                                                    page_io.file_io().update_meta_finish();
                                                });
            ret->indexes.push_back(index);
        }
    }
    ret->setConstraints(cons);
    return ret;
}
