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
#include <sys/time.h>

#include <readline/readline.h>
#include <readline/history.h>

using namespace usql;

static void strip_whitespace(std::string & str) {
    while(!str.empty()) {
        auto c = str.back();
        if(c == ' ' || c == '\r' || c == '\n' || c == '\t')
            str.pop_back();
        else break;
    }
}

static void strip_back(std::string & str, const char c) {
    while(!str.empty() && str.back() == c)
        str.pop_back();
}

static long long get_time() {
    struct timeval tp;
    gettimeofday(&tp, nullptr);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

static std::string read_sql() {
    std::string ret;
    char * line_read = readline("> ");
    if(line_read == nullptr) return ret;
    ret += line_read;
    free(line_read);
    strip_whitespace(ret);
    while(ret.back() != ';') {
        char * line_read = readline("... ");
        if(line_read == nullptr) break;
        ret += " ";
        ret += line_read;
        free(line_read);
        strip_whitespace(ret);
    }
    return ret;
}

static std::string help_msg = "Usage: ./usql [-v] [-d target.usql]";

int main(int argc, char * const argv[]) {

    g_usql_log_enable = false;
    std::string db_dir = "usql.db";
    bool debug_parse = false;
    bool show_timer = false;

    char c;
    while((c = getopt(argc, argv, "vgthd:")) != -1) {
        switch(c) {
            case 'v': g_usql_log_enable = true; break;
            case 'd': db_dir = optarg; break;
            case 'g': debug_parse = true; break;
            case 't': show_timer = true; break;
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

    while(true) {
        std::string sql = read_sql();
        if(sql.size() == 0) break;

        add_history(sql.c_str());
        strip_back(sql, ';');

        auto t0 = get_time();

        auto stmt = std::make_unique<SQLStatement>(sql);
        stmt->setDebug(debug_parse);
        if(stmt->parse() != 0) {
            std::cerr << "Syntax error." << std::endl;
            continue;
        }
        stmt->normalize();

        auto t1 = get_time();

        try {
            runner->run(stmt);
        } catch (USQLError & e) {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }

        auto t2 = get_time();

        if(show_timer) {
            std::cerr << std::endl << "Time: parse " 
                << t1 - t0 << "ms, execution " << t2 - t1 << "ms" << std::endl;
        }
    }

    return 0;
}
