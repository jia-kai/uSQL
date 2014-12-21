/* 
* @Author: BlahGeek
* @Date:   2014-12-21
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#ifndef __usql_runner_h__
#define __usql_runner_h__

#include "./sql_runner.h"

using namespace usql;

namespace usql {

class Runner {
private:
    std::unique_ptr<PageIO> page_io;
    std::unique_ptr<SQLRunner> sql_runner;
    std::string basedir;
    std::string dbname;

    std::string getFileName(std::string db) {
        return basedir + "/" + db + ".usql"; 
    }

protected:
    void createDB(std::string dbname);
    void useDB(std::string dbname);
    void dropDB(std::string dbname);

public:
    using callback_t = SQLRunner::callback_t;

    Runner(std::string dir): basedir(dir) {}
    void run(const std::unique_ptr<SQLStatement> & stmt,
             callback_t callback = SQLRunner::default_callback);
};

}
#endif
