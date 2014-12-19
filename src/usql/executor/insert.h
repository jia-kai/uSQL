/* 
* @Author: BlahGeek
* @Date:   2014-12-18
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-19
*/

#ifndef __usql_insert_executor_h__
#define __usql_insert_executor_h__ value

#include "../sql_statement.h"
#include "../table_info.h"

#include "./base.h"

using namespace usql;

namespace usql {

class InsertException: public std::exception {
private:
    std::string reason;
public:
    InsertException(std::string s): reason(s) {}
    const char * what() const noexcept override {return reason.c_str(); }
};

class InsertExecutor: public BaseExecutor {
public:
    using BaseExecutor::BaseExecutor;
    rowid_t insert(const std::vector<LiteralData> & vals);
};

}

#endif
