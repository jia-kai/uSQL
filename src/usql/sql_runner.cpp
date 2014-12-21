/* 
* @Author: BlahGeek
* @Date:   2014-12-20
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <iostream>
#include <sstream>
#include <algorithm>
#include "./sql_runner.h"

using namespace usql;

SQLRunner::callback_t SQLRunner::default_callback = [](const std::vector<LiteralData> vals) {
    for(size_t i = 0 ; i < vals.size() ; i += 1) {
        if(i != 0) std::cout << "|";
        vals[i].print(std::cout);
    }
    std::cout << std::endl;
};

SQLRunner::SQLRunner(PageIO & page_io): page_io(page_io) {
    root_table = TableInfo::getRootTableInfo(page_io);
}

void SQLRunner::_update(std::string name, std::string col, LiteralData val) {
    usql_log("Updating root table: %s %s", name.c_str(), col.c_str());
    std::unique_ptr<UpdateExecutor> updator (new UpdateExecutor(
                                                {root_table},
                                                {ColumnAndTableName(root_table->name, col)}));
    // where name = name
    auto where_stmt = std::make_unique<WhereStatement>();
    where_stmt->a_is_literal = false;
    where_stmt->na = ColumnAndTableName(root_table->name, "name");
    where_stmt->b = name;
    where_stmt->normalize();

    auto ret = updator->execute({val}, where_stmt);
    usql_assert(ret.size() == 1, "Update root table failed");
    usql_log("Update root table done");
}

void SQLRunner::findTableIndexes(std::shared_ptr<TableInfo> & tableinfo) {
    tableinfo->indexes.clear();
    tableinfo->indexes.resize(tableinfo->table->columns.size());

    std::unique_ptr<SelectExecutor> selector (new SelectExecutor(
                                              {root_table},
                                              {ColumnAndTableName("*", "*")}));

    // desc = tbname
    auto where_stmt = std::make_unique<WhereStatement>();
    where_stmt->a_is_literal = false;
    where_stmt->na = ColumnAndTableName(root_table->name, "desc");
    where_stmt->b = LiteralData(tableinfo->name);
    where_stmt->normalize();

    selector->find(where_stmt, [&, this](rowid_t rowid, const std::vector<LiteralData> & vals) -> bool {
        long long int n = vals[5].int_v;
        usql_log("Found index for table %s: %s (%lld)", 
                 vals[2].string_v.c_str(), vals[1].string_v.c_str(), n);
        auto name = vals[1].string_v;
        tableinfo->indexes[n] = tableinfo->table->columns[n].second->load_index(
                page_io, vals[3].int_v, [name, this](const PageIO::Page & root) {
                    this->_update(name, "page_root", LiteralData(root.id()));
                });
        return false;
    });
}

std::vector<std::string> SQLRunner::getTableNames() {
    std::vector<std::string> ret;
    std::unique_ptr<SelectExecutor> selector (new SelectExecutor(
                                              {root_table},
                                              {ColumnAndTableName(root_table->name, "name")}));

    auto where_stmt = std::make_unique<WhereStatement>();
    where_stmt->a_is_literal = false;
    where_stmt->na = ColumnAndTableName(root_table->name, "type");
    where_stmt->b = LiteralData(int(TableInfo::RootRowType::TABLE));
    where_stmt->normalize();

    selector->find(where_stmt, [&](rowid_t rowid, const std::vector<LiteralData> & vals) -> bool {
        ret.push_back(vals[0].string_v);
        return false;
    });
    return ret;
}

std::shared_ptr<TableInfo> SQLRunner::getTableInfo(std::string tbname) {
    auto ret = std::make_shared<TableInfo>();
    ret->name = tbname;

    std::unique_ptr<SelectExecutor> selector(new SelectExecutor(
                                             {root_table},
                                             {ColumnAndTableName("*", "*")}));
    // name = tbname and type = TABLE
    auto where_stmt = std::make_unique<WhereStatement>();
    where_stmt->type = WhereStatement::WhereStatementType::AND;
    
    where_stmt->children.clear();
    where_stmt->children.push_back(std::make_unique<WhereStatement>());
    where_stmt->children.push_back(std::make_unique<WhereStatement>());

    where_stmt->children[0]->a_is_literal = false;
    where_stmt->children[0]->na = ColumnAndTableName(root_table->name, "name");
    where_stmt->children[0]->b = LiteralData(tbname);

    where_stmt->children[1]->a_is_literal = false;
    where_stmt->children[1]->na = ColumnAndTableName(root_table->name, "type");
    where_stmt->children[1]->b = LiteralData(int(TableInfo::RootRowType::TABLE));

    where_stmt->normalize();

    selector->find(where_stmt, [&, this](rowid_t rowid, const std::vector<LiteralData> & vals) -> bool{
        usql_assert(ret->table == nullptr, "table already found!");
        usql_log("Found table %s: %s", vals[1].string_v.c_str(), vals[2].string_v.c_str());
        SQLStatement schema_stmt(vals[2].string_v);
        auto err = schema_stmt.parse();
        usql_assert(err == 0, "parse schema error");
        schema_stmt.normalize();
        ret->table = std::make_shared<Table>(page_io, schema_stmt.column_defs,
                                             vals[4].int_v, [tbname, this](rowid_t newid) {
                                                this->_update(tbname, "rows", LiteralData(newid));
                                             });
        ret->table->load(vals[3].int_v, [tbname, this](const PageIO::Page & p) {
            this->_update(tbname, "page_root", LiteralData(p.id()));
        });
        ret->setConstraints(schema_stmt.column_constraints);
        ret->desc = vals[2].string_v;
        return false;
    });

    if(ret->table == nullptr) return nullptr;

    this->findTableIndexes(ret);
    return ret;

}

void SQLRunner::createTable(std::string tbname, const std::vector<column_def_t> & cols, std::string sql) {
    usql_log("Creating table %s: %s", tbname.c_str(), sql.c_str());
    std::unique_ptr<InsertExecutor> exe(new InsertExecutor({root_table}, {
        ColumnAndTableName(root_table->name, "type"),
        ColumnAndTableName(root_table->name, "name"),
        ColumnAndTableName(root_table->name, "desc"),
    }));
    try {
        exe->insert({
            LiteralData(int(TableInfo::RootRowType::TABLE)),
            LiteralData(tbname),
            LiteralData(sql)
        });
    } catch (NotUniqueException & e) {
        throw SQLException("Table already exists");
    }

    auto tableinfo = this->getTableInfo(tbname);
    usql_assert(tableinfo->name == tbname, "Create table failed");

    for(size_t i = 0 ; i < tableinfo->constraints.size() ; i += 1) {
        auto & cons = tableinfo->constraints[i];
        if(cons.find(SQLStatement::ColumnConstraint::PRIMARY) != cons.end()
           || cons.find(SQLStatement::ColumnConstraint::UNIQUE) != cons.end()) 
            this->createIndex(tableinfo->table, 
                              ColumnAndTableName(tbname, cols[i].first),
                              i);
    }
}

void SQLRunner::createIndex(const std::shared_ptr<Table> & table, ColumnAndTableName name, int col_index) {
    if(col_index == -1) {
        auto it = std::find_if(table->columns.begin(), table->columns.end(),
                               [&](const column_def_t & col) -> bool {
                                return col.first == name.second;
                               });
        usql_assert(it != table->columns.end(), "Column not found: %s", name.second.c_str());
        col_index = it - table->columns.begin();
    }
    usql_log("Creating index %s.%s (%d)", 
             name.first.c_str(), name.second.c_str(), col_index);

    size_t root_page = 0;
    auto index = table->columns[col_index].second->load_index(
            page_io, root_page, [&](const PageIO::Page & root) {
                root_page = root.id();
            });
    table->walkthrough([&](rowid_t rowid, const std::vector<LiteralData> & vals) -> bool {
        index->insert(vals[col_index], rowid);
        return false;
    });

    std::unique_ptr<InsertExecutor> exe(new InsertExecutor({root_table}, {
        ColumnAndTableName(root_table->name, "type"),
        ColumnAndTableName(root_table->name, "name"),
        ColumnAndTableName(root_table->name, "desc"),
        ColumnAndTableName(root_table->name, "page_root"),
        ColumnAndTableName(root_table->name, "col_index")
    }));
    try {
        exe->insert({
            LiteralData(int(TableInfo::RootRowType::INDEX)),
            LiteralData(name.first + "." + name.second),
            LiteralData(name.first),
            LiteralData(root_page),
            LiteralData(col_index)
        });
    } catch(NotUniqueException & e) {
        throw SQLException("Index already exists");
    }
}

void SQLRunner::run(const std::unique_ptr<SQLStatement> & stmt,
                    SQLRunner::callback_t callback) {
    std::vector<std::shared_ptr<TableInfo>> tableinfos;
    if(stmt->type != SQLStatement::Type::CREATE_TB)
        for(auto & name: stmt->table_names) {
            auto tableinfo = this->getTableInfo(name);
            if(tableinfo == nullptr)
                throw SQLException("Table not found");
            tableinfos.push_back(tableinfo);
        }

    if(stmt->type == SQLStatement::Type::INSERT) {
        usql_log("Type is INSERT, calling InsertExecutor");
        auto exe = std::make_unique<InsertExecutor>(tableinfos, stmt->column_names);
        for(auto & vals: stmt->values)
            exe->insert(vals);
        return;
    }
    if(stmt->type == SQLStatement::Type::SELECT) {
        usql_log("Type is SELECT, calling SelectExecutor");
        auto exe = std::make_unique<SelectExecutor>(tableinfos, stmt->column_names);
        exe->find(stmt->where_stmt, 
                  [&](rowid_t rowid, const std::vector<LiteralData> & vals) -> bool {
                    if(callback) callback(vals); 
                    return false;
                  });
        return;
    }
    if(stmt->type == SQLStatement::Type::DELETE) {
        usql_log("Type is DELETE, calling DeleteExecutor");
        auto exe = std::make_unique<DeleteExecutor>(tableinfos);
        exe->execute(stmt->where_stmt);
        return;
    }
    if(stmt->type == SQLStatement::Type::UPDATE) {
        usql_log("Type is UPDATE, calling UpdateExecutor");
        auto exe = std::make_unique<UpdateExecutor>(tableinfos, stmt->column_names);
        exe->execute(stmt->values.back(), stmt->where_stmt);
        return;
    }
    if(stmt->type == SQLStatement::Type::CREATE_TB) {
        std::ostringstream sql;
        stmt->print(sql);
        this->createTable(stmt->table_names.back(), 
                          stmt->column_defs,
                          sql.str());
        return;
    }
    if(stmt->type == SQLStatement::Type::CREATE_IDX) {
        for(auto & name: stmt->column_names)
            this->createIndex(tableinfos[0]->table, name, -1);
        return;
    }
    if(stmt->type == SQLStatement::Type::SHOW_TBS) {
        auto tables = this->getTableNames();
        for(auto & name: tables) {
            auto tableinfo = this->getTableInfo(name);
            if(callback == nullptr) continue;
            callback(std::vector<LiteralData>({
                LiteralData(name), LiteralData(tableinfo->desc)
            }));
        }
        return;
    }
    throw SQLException("Not implemented yet");
}
