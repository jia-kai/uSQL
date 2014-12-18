/* 
* @Author: BlahGeek
* @Date:   2014-12-15
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-18
*/


#ifndef __usql_test_table_and_index_env_h__
#define __usql_test_table_and_index_env_h__ value

#include "./page_io_env.h"
#include "usql/table.h"
#include "usql/index.h"
#include "usql/sql_statement.h"

using namespace usql;

class TableAndIndexEnv: public PageIOTestEnv {
protected:
    std::shared_ptr<Table> table0, table1;
    std::shared_ptr<IndexBase> t0_c1_index, t1_c0_index;

    PageIO::page_id_t table0_root = 0, table1_root = 0;
    PageIO::page_id_t t0_c1_index_root = 0, t1_c0_index_root = 0;

    rowid_t table0_rows = 0, table1_rows = 0;

    void SetUp() override {
        PageIOTestEnv::SetUp();
        // create table0
        std::string sql0("create table table0"
                   "(c0 int, c1 int unique, c2 varchar(128) NOT NULL)");
        SQLStatement stmt0(sql0);
        usql_assert(0 == stmt0.parse(), "table0 parse error");
        stmt0.normalize();
        table0 = std::make_shared<Table>(*m_page_io, stmt0.column_defs,
                                    stmt0.column_constraints,
                                    table0_rows, 
                                    [this](const rowid_t r){table0_rows = r; });
        table0->load(table0_root, 
                     [this](const PageIO::Page & root){ table0_root = root.id(); });
        // create table1
        std::string sql1("create table table1"
                   "(c0 varchar(16), c1 int)");
        SQLStatement stmt1(sql1);
        usql_assert(0 == stmt1.parse(), "table1 parse error");
        stmt1.normalize();
        table1 = std::make_shared<Table>(*m_page_io, stmt1.column_defs,
                                    stmt1.column_constraints,
                                    table1_rows, 
                                    [this](const rowid_t r){table1_rows = r; });
        table1->load(table1_root, 
                     [this](const PageIO::Page & root){ table1_root = root.id(); });
        // create index for table0.c1
        t0_c1_index = table0->columns[1].second->load_index(
                     *m_page_io, t0_c1_index_root,
                     [this](const PageIO::Page & root) {t0_c1_index_root = root.id(); });
        // create index for table1.c0
        t1_c0_index = table1->columns[0].second->load_index(
                     *m_page_io, t1_c0_index_root,
                     [this](const PageIO::Page & root) {t1_c0_index_root = root.id(); });
        // insert values to table0
        for(int i = 0 ; i < 10000 ; i += 1) {
            std::vector<LiteralData> vals = {
                LiteralData(i), LiteralData(-i), 
                LiteralData(ssprintf("No. %d", i))
            };
            auto row = table0->insert(vals);
            t0_c1_index->insert(vals[1], row);
        }
        // insert values to table1
        for(int i = 0 ; i < 1000 ; i += 1) {
            std::vector<LiteralData> vals = {
                LiteralData(ssprintf("%d", i)), LiteralData(i % 100)
            };
            auto row = table1->insert(vals);
            t1_c0_index->insert(vals[0], row);
        }
    }
};

#endif
