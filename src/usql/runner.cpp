/* 
* @Author: BlahGeek
* @Date:   2014-12-21
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <iostream>
#include "./runner.h"
#include <unistd.h>

using namespace usql;

#define PAGE_SIZE 8192

void Runner::useDB(std::string dbname) {
    auto filename = getFileName(dbname);
    if(access(filename.c_str(), R_OK | W_OK))
        throw SQLException("Database not exists");
    this->dbname = dbname;
    page_io = std::make_unique<PageIO>(
        FileIO{filename.c_str(), PAGE_SIZE});
    sql_runner = std::make_unique<SQLRunner>(*page_io);
}

void Runner::createDB(std::string dbname) {
    auto filename = getFileName(dbname);
    if(access(filename.c_str(), R_OK | W_OK) == 0)
        throw SQLException("Database already exists");
    this->dbname = dbname;
    page_io = std::make_unique<PageIO>(
        FileIO{filename.c_str(), PAGE_SIZE});
    sql_runner = std::make_unique<SQLRunner>(*page_io);
}

void Runner::dropDB(std::string dbname) {
    auto filename = getFileName(dbname);
    if(dbname == this->dbname) {
        sql_runner = nullptr;
        page_io = nullptr;
    }
    unlink(filename.c_str());
}

void Runner::run(const std::unique_ptr<SQLStatement> & stmt,
                 Runner::callback_t callback) {
    if(stmt->type == SQLStatement::Type::CREATE_DB) {
        this->createDB(stmt->database_name);
        return;
    }
    if(stmt->type == SQLStatement::Type::DROP_DB) {
        this->dropDB(stmt->database_name);
        return;
    }
    if(stmt->type == SQLStatement::Type::USE_DB) {
        this->useDB(stmt->database_name);
        return;
    }

    if(sql_runner == nullptr)
        throw SQLException("No valid database selected");
    sql_runner->run(stmt, callback);
}
