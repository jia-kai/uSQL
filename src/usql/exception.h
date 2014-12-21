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

class USQLError: public std::exception {
    std::string m_reason;

    public:
        USQLError(const std::string &reason):
            m_reason{reason}
        {}

        const char * what() const noexcept override {
            return m_reason.c_str();
        }
};

class SQLException: public USQLError {
    public:
        using USQLError::USQLError;
};

class AssertionError: public USQLError {
    public:
        using USQLError::USQLError;
};

}

#endif
