/* 
* @Author: BlahGeek
* @Date:   2014-12-21
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <iostream>
#include "./page_io_env.h"

#include "usql/sql_runner.h"

using namespace usql;

class SQLRunnerTest: public PageIOTestEnv {
protected:
    std::unique_ptr<SQLRunner> runner;
    void SetUp() override {
        PageIOTestEnv::SetUp();
        runner = std::make_unique<SQLRunner>(*m_page_io);
    }

    void run(std::string sql, SQLRunner::callback_t callback = nullptr) {
        auto stmt = std::make_unique<SQLStatement>(sql);
        // stmt->setDebug(true);
        EXPECT_EQ(stmt->parse(), 0);
        stmt->normalize();
        stmt->print(std::cerr);
        std::cerr << std::endl;

        if(callback == nullptr) runner->run(stmt);
        else runner->run(stmt, callback);
    }
};

TEST_F(SQLRunnerTest, basic) {
    run("CREATE TABLE table0 (c0 INT, c1 INT PRIMARY KEY, c2 VARCHAR(23) UNIQUE)");
    run("INSERT INTO table0 (c0, c1) values (1,2)");
    run("SELECT * FROM table0");
    run("CREATE INDEX ON table0 (c0)");
    ASSERT_ANY_THROW(run("CREATE INDEX ON table0 (c0)"));
}
