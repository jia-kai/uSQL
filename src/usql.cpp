/* 
* @Author: BlahGeek
* @Date:   2014-12-21
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <iostream>
#include "./usql/runner.h"
#include "./usql/parser/sql_statement.h"

using namespace usql;

static std::string welcome_msg = "Welcome to uSQL 0.0.1";

int main(int argc, char const *argv[]) {
    if(argc < 2) {
        std::cout << "Usage: " << argv[0] << " [DIR]" << std::endl;
        exit(0);
    }
    std::cout << welcome_msg << std::endl;

    auto runner = std::make_unique<Runner>(std::string(argv[1]));
    while(std::cin.good()) {
        std::string sql;
        std::cout << "> ";
        std::getline(std::cin, sql);
        if(!std::cin.good())
            break;
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