/* 
* @Author: BlahGeek
* @Date:   2014-12-21
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-29
*/

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include "./usql/runner.h"
#include "./usql/common.h"
#include "./usql/parser/sql_statement.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <readline/readline.h>
#include <readline/history.h>

using namespace usql;

static std::string help_msg = "Usage: ./usql [-v] [-d target.usql]";

int main(int argc, char * const argv[]) {

    g_usql_log_enable = false;
    std::string db_dir = "usql.db";

    char c;
    while((c = getopt(argc, argv, "vhd:")) != -1) {
        switch(c) {
            case 'v': g_usql_log_enable = true; break;
            case 'd': db_dir = optarg; break;
            case 'h': std::cerr << help_msg << std::endl; exit(1);
            default: std::cerr << "Unknown option." << std::endl << help_msg << std::endl; exit(1);
        }
    }
    std::cerr << "Welcome to uSQL 0.0.1, using " << db_dir << std::endl;

    struct stat db_dir_info;
    if(stat(db_dir.c_str(), &db_dir_info) != 0 || !(db_dir_info.st_mode & S_IFDIR)) {
        std::cerr << db_dir << " is not a directory." << std::endl;
        exit(1);
    }

    auto runner = std::make_unique<Runner>(db_dir);

    while(char * line_read = readline("> ")) {
        add_history(line_read);
        std::string sql(line_read);
        free(line_read);

        // remove trailing ';'
        while(sql.back() == ';')
            sql.pop_back();

        auto stmt = std::make_unique<SQLStatement>(sql);
        if(stmt->parse() != 0) {
            std::cerr << "Syntax error." << std::endl;
            continue;
        }
        stmt->normalize();
        try {
            runner->run(stmt);
        } catch (SQLException & e) {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }
    }

    return 0;
}
