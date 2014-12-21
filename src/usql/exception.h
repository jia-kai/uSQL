/* 
* @Author: BlahGeek
* @Date:   2014-12-21
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/
#ifndef __usql_exception_h__
#define __usql_exception_h__  

#include <exception>
#include "./common.h"

using namespace usql;

namespace usql {

class SQLException: public std::exception {
private:
    std::string reason;
public:
    SQLException(std::string s): reason(s) {}
    virtual const char * what() const noexcept override {return reason.c_str(); }
};

}

#endif
